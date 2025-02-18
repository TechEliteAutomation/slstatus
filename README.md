# slstatus
This customized version of **slstatus** displays a lightweight status bar for X11 that supports a wide range of modules, including weather, time of day, wind direction, and more. It updates the information displayed in the status bar at specified intervals and supports system resource monitoring.

## Features

- **Weather Information:** Fetches current weather data, including temperature, humidity, wind speed, and visibility using the [OpenWeatherMap API](https://openweathermap.org/).
- **Time of Day:** Displays the current phase of the day (e.g., morning, afternoon, sunset) based on sunrise and sunset data fetched from [SunriseSunset.io](https://sunrisesunset.io/).
- **Wind Direction:** Displays the wind direction using Unicode arrow symbols.
- **Customizable Update Intervals:** Configurable update intervals (default: 1 second).
- **Error Handling:** Displays a fallback value (`␀`) when data is unavailable.

## Configuration

The status bar updates every second by default, and it uses several modules to fetch and display dynamic information. The `config.h` file allows for customization of these modules.

### Modules

- **Weather:** Fetches weather data for the specified location using the OpenWeatherMap API. To use this feature, ensure that the `OPENWEATHER_API_KEY` environment variable is set.
- **Time of Day:** Displays the time of day based on sunrise, sunset, and other daily events like dawn and dusk. The times are fetched from the SunriseSunset.io API.
- **Wind Direction:** Displays the wind direction as an arrow based on the current wind degree.

### Required Environment Variables

- **OPENWEATHER_API_KEY:** You must set the `OPENWEATHER_API_KEY` environment variable in your shell configuration (`~/.bashrc`, `~/.zshrc`, etc.) to use the weather module.

Example for `~/.bashrc`:
```bash
export OPENWEATHER_API_KEY="your_api_key_here"
```

### Build Instructions

1. Clone the repository:
    ```bash
    git clone https://github.com/your-username/slstatus.git
    cd slstatus
    ```

2. Build the program:
    ```bash
    make
    ```

3. Install (optional):
    ```bash
    sudo make install
    ```

### Usage

Run `slstatus` in your X11 environment. It will update the status bar every second with the current time, weather, and wind direction.

### License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for more details.

## Example Output

- **Weather:** `🌞 75°F (Clear, 10 mph)`
- **Time of Day:** `🌅 morning`
- **Wind Direction:** `→`

## Configuration File

You can modify the behavior of the status bar by editing `config.h`. Here are the main configurations:

- `interval`: Set the update interval (in milliseconds).
- `unknown_str`: Set the string to display when no value is available.
- `fetch_time_of_day`: Defines how time-of-day events (like sunrise, sunset) are calculated.
- `weather`: Fetches and displays weather data based on your location and OpenWeatherMap API key.

## Troubleshooting

- If you get errors regarding the OpenWeatherMap API key, ensure that the `OPENWEATHER_API_KEY` environment variable is set properly.
- If data is not displaying correctly, ensure that your system has `curl` installed and can reach external APIs.
