# Tweaker Settings

This section contains entries that affect GP4 Tweaker's behaviour and features. Currently, there is only one entry for overriding the brake light behaviour.

```ini
[Tweaker]
OverrideBrakeLight = false
```

## Override Brake Light

When using GP4 Tweaker, in order for the `RearAmbientLight` tweak to work, the `RearLightBrake` tweak also needs to be enabled. Whether a bug of GP4 Tweaker or an intended behaviour at the time, there are situations, like in current F1 cars, where multiple ambient lights might be desired without necessarily having them all behave like brake lights as well.

`OverrideBrakeLight` allows to decouple the brake light behaviour from the rear ambient light, when set to `true`/`1`, by restorting the original GP4 behaviour of the rear ambient light.

All settings described in the [Rear Light Settings](Rear-Light-Settings.md) section will still be applied.
