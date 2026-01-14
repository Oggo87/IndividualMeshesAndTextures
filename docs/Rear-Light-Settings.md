# Rear Light Settings

This section contains to control the bhaviour of the rear light(s). All options can be combined as desired, and the rear light will transition seamlessly between the different states and the regular "off" state. These states are:

* [Wet Weather](#Wet-Weather)
* [Brake](#Brake)
* [Charge ERS](#Charge-ERS)

## Wet Weather

These settings allow to control the behaviour of the rear light in case of wet weather.

### WetWeather

This setting enables the rear light to be turned on or off in case of wet track. The entry can be set to `true` / `false` or `1` / `0`.

### WetWetherBlinking

This setting enables blinking of the rear light in case of wet weather. The entry can be set to `true` / `false` or `1` / `0`.

### WetWeatherPeriod

This setting controls the blinking period of the rear light in case of wet weather. The entry is specified in milliseconds.

Example - Rear light on in case of wet weather, blinking, with a period of 250ms (4Hz)

```ini
[RearLight]
WetWeather = true
WetWeatherBlinking = true
WetWeatherPeriod = 250
```

## Brake

These settings allow to control the behaviour of the rear light when braking. When enabled alongside GP4 Tweaker's Rear Light Brake tweak, the two will work in conjunction. It is recommended to disable Rear Light Brake in GP4 Tweaker when using this feature.

### Brake

This setting enables the rear light to be turned on or off while braking. The entry can be set to `true` / `false` or `1` / `0`.

### BrakeThreshold

This setting controls the braking threshold (in percentage of maximum braking force) above which the rear light will turn on. The entry is specified as an integer value between 0 and 100.

Example - Rear light on when braking above 5% of maximum braking force

```ini
[RearLight]
Brake = true
BrakeThreshold = 5
```

## Pit Limiter

These settings allow to control the behaviour of the rear light when the pit limiter is active. When enabled alongisde GP4 Tweaker's Pit Limiter tweak,GP4++ will detect the pit limiter status set by GP4 Tweaker.

### PitLimiter

This setting enables the rear light to be turned on or off when the pit limiter is active. The entry can be set to `true` / `false` or `1` / `0`.

### PitLimiterBlinking

This setting enables blinking of the rear light when the pit limiter is active. The entry can be set to `true` / `false` or `1` / `0`.

### PitLimiterPeriod

This setting controls the blinking period of the rear light when the pit limiter is active. The entry is specified in milliseconds.

Example - Rear light on when pit limiter is active, blinking, with a period of 500ms (2Hz)

```ini
[RearLight]
PitLimiter = true
PitLimiterBlinking = true
PitLimiterPeriod = 500
```

## Charge ERS

These settings allow to control the behaviour of the rear light while simulating the charging of the ERS battery. ERS charging systems are very complex, so a simple simulation is used here, based on the car's speed, throttle and braking inputs.

### ChargeERS

This setting enables the rear light to be turned on or off while simulating the charging of the ERS battery. The entry can be set to `true` / `false` or `1` / `0`.

### ChargeERSBlinking

This setting enables blinking of the rear light while simulating the charging of the ERS battery. The entry can be set to `true` / `false` or `1` / `0`.

### ChargeERSPeriod

This setting controls the blinking period of the rear light while simulating the charging of the ERS battery. The entry is specified in milliseconds.

Example - Rear light on while simulating ERS charging, blinking, with a period of 500ms (2Hz)

```ini
[RearLight]
ChargeERS = true
ChargeERSBlinking = true
ChargeERSPeriod = 500
```