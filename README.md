# zmk-input-zoom

A [ZMK](https://zmk.dev) module that provides an input processor converting relative events into key-press events. This project was developed with the assistance of AI coding tools.

This processor watches for a configurable relative input event. When the delta is positive, it fires the zoom-in binding. When the delta is negative, it fires the zoom-out binding.

## Usage

### 1. Define the processor node

```dts
#include <input/input_event_codes.h>

/ {
    zip_zoom_mapper: zip_zoom_mapper {
        compatible = "zmk,input-processor-zoom";
        #input-processor-cells = <0>;
        event-code = <INPUT_REL_MISC>;
        sensitivity = <1>;
        bindings = <&kp LG(MINUS)>, <&kp LG(EQUAL)>;
        track-remainders;
    };
};
```

Alternatively, use `<&kp LC(MINUS)>, <&kp LC(EQUAL)>` for the control key instead of command key.

| Property | Required | Description |
| --- | --- | --- |
| `event-code` | yes | Relative event code to match (e.g. `INPUT_REL_MISC`, `INPUT_REL_WHEEL`) |
| `bindings` | yes | Two behaviors: `[zoom-out, zoom-in]` fired on negative / positive delta |
| `sensitivity` | no | Delta units per zoom step (default `1`) |
| `track-remainders` | no | Accumulate sub-threshold deltas across events (boolean flag; omit to disable) |

### 2. Attach to an input processor

```dts
&my_trackpad {
  input-processors = <&zip_zoom_mapper>;
};
```

Or compose with other processors:

```dts
&my_trackpad {
  input-processors = <&zip_xy_scaler 2 1>, <&zip_zoom_mapper>;
};
```
