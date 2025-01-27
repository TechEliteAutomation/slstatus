#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h> /* for usleep */

#define MAXLEN 2048

/* interval between updates (in ms) */
const unsigned int interval = 1000; // Update every second

/* text to show if no value can be retrieved */
static const char unknown_str[] = "␀";

static const char *
get_wind_direction_arrow(int degrees) {
    // Rotate the degree by 180° to indicate the flowing direction
    int flowing_degrees = (degrees + 180) % 360;

    static const char *arrows[] = {
        "↑", "↗", "↗", "↗",
        "→", "↘", "↘", "↘",
        "↓", "↙", "↙", "↙",
        "←", "↖", "↖", "↖"
    };
    return arrows[((flowing_degrees + 11) % 360) / 22];
}

static int
is_within_one_minute(time_t current, time_t target) {
    return abs((int)difftime(current, target)) <= 60;
}

static const char *
fetch_time_of_day(const char *unused) {
    static char buf[MAXLEN] = "␀";         // Cached event description
    static time_t parsed_times[7] = {0};  // Cached sunrise/sunset times
    static int last_date = -1;            // Cached date (day of the year)
    const char *keys[] = {"first_light", "dawn", "sunrise", "solar_noon", "sunset", "dusk", "last_light"};
    struct tm *current_tm;
    
    time_t now = time(NULL);
    current_tm = localtime(&now);
    int current_date = current_tm->tm_yday; // Current day of the year

    // Check if data needs to be updated
    if (last_date != current_date) {
        const char *url = "https://api.sunrisesunset.io/json?lat=40.089470&lng=-74.039214";
        char cmd[MAXLEN];
        snprintf(cmd, sizeof(cmd), "curl -s '%s'", url);

        FILE *fp = popen(cmd, "r");
        if (fp == NULL) {
            return buf; // Return cached or default if curl fails
        }

        char response[MAXLEN];
        if (fgets(response, sizeof(response) - 1, fp) == NULL) {
            pclose(fp);
            return buf; // Return cached or default if API fails
        }
        pclose(fp);

        // Parse times from response
        for (int i = 0; i < 7; i++) {
            char search_str[64];
            snprintf(search_str, sizeof(search_str), "\"%s\":\"", keys[i]);
            char *time_str = strstr(response, search_str);
            if (time_str) {
                time_str += strlen(search_str);
                char hour[3], min[3], sec[3], ampm[3];
                sscanf(time_str, "%2[0-9]:%2[0-9]:%2[0-9] %2[AMP]M", hour, min, sec, ampm);

                struct tm time_components = {0};
                time_components.tm_hour = atoi(hour);
                if (strcmp(ampm, "PM") == 0 && time_components.tm_hour != 12)
                    time_components.tm_hour += 12;
                else if (strcmp(ampm, "AM") == 0 && time_components.tm_hour == 12)
                    time_components.tm_hour = 0;
                time_components.tm_min = atoi(min);
                time_components.tm_sec = atoi(sec);
                time_components.tm_year = current_tm->tm_year;
                time_components.tm_mon = current_tm->tm_mon;
                time_components.tm_mday = current_tm->tm_mday;

                parsed_times[i] = mktime(&time_components);
            }
        }
        last_date = current_date; // Update cached date
    }

    // Determine current event
    const char *event = "unknown";
    if (now < parsed_times[0]) event = 		"🌌night";
    else if (now < parsed_times[1]) event = "🌃twilight";
    else if (now < parsed_times[2]) event = "🌄sunrise";
    else if (now < parsed_times[3]) event = "🌅morning";
    else if (now < parsed_times[4]) event = "🔆afternoon";
    else if (now < parsed_times[5]) event = "🌄sunset";
    else if (now < parsed_times[6]) event = "🌆evening";
    else event = "🌌night";

    snprintf(buf, sizeof(buf), "%s", event);
    return buf;
}

static const char *
weather(const char *arg) {
    static char buf[MAXLEN] = "␀"; // Cached data
    static time_t last_update = 0; // Last update time
    const int update_interval = 900; // Update every 15 minutes

    time_t now = time(NULL);
    if (difftime(now, last_update) < update_interval) {
        return buf; // Return cached result
    }

    //"OPENWEATHER_API_KEY" IS DEFINED IN ~/.bashrc
	const char *api_key = getenv("OPENWEATHER_API_KEY");
		if (api_key == NULL) {
			fprintf(stderr, "Error: OPENWEATHER_API_KEY environment variable not set\n");
			return buf; // Return cached result if environment variable is not set
		}
	const char *latitude = "40.08947098125882";
    const char *longitude = "-74.0392148066487";
    const char *url_template = 
        "curl -s 'https://api.openweathermap.org/data/2.5/weather?lat=%s&lon=%s&appid=%s&units=imperial'";
    char url[MAXLEN];
    snprintf(url, sizeof(url), url_template, latitude, longitude, api_key);

    FILE *fp = popen(url, "r");
    if (fp == NULL) {
        return buf; // Return cached or default if curl fails
    }

    char response[MAXLEN];
    if (fgets(response, sizeof(response) - 1, fp) == NULL) {
        pclose(fp);
        return buf;
    }
    pclose(fp);

    // Extract weather data
    char *temp_ptr = strstr(response, "\"temp\":");
    char *feels_like_ptr = strstr(response, "\"feels_like\":");
    char *wind_speed_ptr = strstr(response, "\"speed\":");
    char *wind_deg_ptr = strstr(response, "\"deg\":");
    char *humidity_ptr = strstr(response, "\"humidity\":");
    char *visibility_ptr = strstr(response, "\"visibility\":");
    char *pressure_ptr = strstr(response, "\"pressure\":");
    char *icon_ptr = strstr(response, "\"icon\":\"");

    if (temp_ptr && feels_like_ptr && wind_speed_ptr && wind_deg_ptr && 
        humidity_ptr && visibility_ptr && pressure_ptr && icon_ptr) {

        float temp = atof(temp_ptr + 7);
        float feels_like = atof(feels_like_ptr + 13);
        float wind_speed = atof(wind_speed_ptr + 8);
        int wind_deg = atoi(wind_deg_ptr + 6);
        int humidity = atoi(humidity_ptr + 11);
        float visibility = atof(visibility_ptr + 13) / 1000.0; // Convert to km
        int pressure = atoi(pressure_ptr + 11);

        char icon[5];
        strncpy(icon, icon_ptr + 8, 3);
        icon[3] = '\0'; // Null-terminate the string

		// Map the OpenWeatherMap icon code to a Unicode weather icon with descriptions
		const char *icon_str;
		const char *description;

		if (strcmp(icon, "01d") == 0) {
			icon_str = "🌞";
			description = "clear";
		} else if (strcmp(icon, "01n") == 0) {
			icon_str = "🌙";
			description = "clear";
		} else if (strcmp(icon, "02d") == 0 || strcmp(icon, "02n") == 0) {
			icon_str = "🌤";
			description = "partly cloudy";
		} else if (strcmp(icon, "03d") == 0 || strcmp(icon, "03n") == 0) {
			icon_str = "☁️";
			description = "cloudy";
		} else if (strcmp(icon, "04d") == 0 || strcmp(icon, "04n") == 0) {
			icon_str = "🌥";
			description = "overcast";
		} else if (strcmp(icon, "09d") == 0 || strcmp(icon, "09n") == 0) {
			icon_str = "🌧";
			description = "rain showers";
		} else if (strcmp(icon, "10d") == 0 || strcmp(icon, "10n") == 0) {
			icon_str = "🌦";
			description = "rainy";
		} else if (strcmp(icon, "11d") == 0 || strcmp(icon, "11n") == 0) {
			icon_str = "🌩";
			description = "thunderstorm";
		} else if (strcmp(icon, "13d") == 0 || strcmp(icon, "13n") == 0) {
			icon_str = "❄️";
			description = "snowy";
		} else if (strcmp(icon, "50d") == 0 || strcmp(icon, "50n") == 0) {
			icon_str = "🌫";
			description = "foggy";
		} else {
			icon_str = "❓";
			description = "unknown";
		}
		
		// Combine icon and description
		char icon_with_description[MAXLEN];
		snprintf(icon_with_description, sizeof(icon_with_description), "%s%s", icon_str, description);

        // Get wind direction arrow
        const char *wind_arrow = get_wind_direction_arrow(wind_deg);

        // Format the weather information
        snprintf(buf, sizeof(buf), "%.1f🌡%.1f 🌀%s%.1f 💧%d 👁%.1f ⬆️%d %s",
                 temp,        			// Temperature
                 feels_like,  			// Feels like temperature
                 wind_arrow,  			// Wind direction
                 wind_speed,  			// Wind speed
                 humidity,    			// Humidity percentage
                 visibility,   			// Visibility in km
                 pressure,				// Pressure in hPa
                 icon_with_description	// Weather icon
                );
    } else {
        strncpy(buf, "Weather: N/A", sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0'; // Ensure null termination
    }

    last_update = now; // Update cache timestamp
    return buf;
}

/* Function to generate battery status with icon - for laptop only
static const char *
battery_status(const char *battery) {
    static char buf[MAXLEN];
    const char *perc_str = battery_perc(battery);
    int perc = perc_str ? atoi(perc_str) : -1;            

    const char *icon;
    if (perc > 75)
        icon = "🔋";
    else if (perc > 37)
        icon = "🔋";
    else if (perc > 4)
        icon = "🪫";
    else if (perc >= 0)
        icon = "❗";
    else
        icon = "❓";
    
    snprintf(buf, sizeof(buf), "%s%s", icon, perc_str ? perc_str : "N/A");
    return buf;
} */

static const struct arg args[] = {
    /* function format              argument */
    { cpu_perc,        		"%s🖥️️",	NULL },
    { ram_perc,        		"%s " , NULL },
    { wifi_perc,       		"%s📶", "wlan0" },
    { wifi_essid,      		"%s " , "wlan0" },
    { weather,         		"%s " , NULL }, // Displays the weather
    { fetch_time_of_day, 	"%s " , NULL }, // Displays time of day
    { datetime, 			"%s " , "%a %m/%d/%y %I:%M:%S %p" },
    /* { battery_status,	"%s " , "BAT0" }, */
};
