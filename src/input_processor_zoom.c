#define DT_DRV_COMPAT zmk_input_processor_zoom

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/input/input.h>
#include <drivers/input_processor.h>
#include <zmk/behavior.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

struct zip_zoom_config {
    uint16_t event_code;
    int32_t sensitivity;
    bool track_remainders;
    struct zmk_behavior_binding zoom_out; /* bindings[0]: delta < 0 */
    struct zmk_behavior_binding zoom_in;  /* bindings[1]: delta > 0 */
};

struct zip_zoom_data {
    int32_t remainder;
};

static int zoom_handle_event(const struct device *dev, struct input_event *event,
                             uint32_t param1, uint32_t param2,
                             struct zmk_input_processor_state *state) {
    const struct zip_zoom_config *cfg = dev->config;
    struct zip_zoom_data *data = dev->data;

    if (event->type != INPUT_EV_REL || event->code != cfg->event_code) {
        return 0;
    }

    int32_t delta = (int32_t)event->value;
    if (delta == 0) {
        return 0;
    }

    int32_t steps;
    if (cfg->track_remainders) {
        data->remainder += delta;
        steps = data->remainder / cfg->sensitivity;
        data->remainder %= cfg->sensitivity;
    } else {
        steps = delta / cfg->sensitivity;
    }

    if (steps == 0) {
        return 0;
    }

    const struct zmk_behavior_binding *binding =
        (steps > 0) ? &cfg->zoom_in : &cfg->zoom_out;
    int32_t count = (steps > 0) ? steps : -steps;

    for (int32_t i = 0; i < count; i++) {
        struct zmk_behavior_binding_event bev = {
            .position = INT32_MAX,
            .timestamp = k_uptime_get(),
        };
        zmk_behavior_invoke_binding(binding, bev, true);
        zmk_behavior_invoke_binding(binding, bev, false);
    }

    event->value = 0;
    return 0;
}

static const struct zmk_input_processor_driver_api zoom_driver_api = {
    .handle_event = zoom_handle_event,
};

#define BINDING_AT(n, idx)                                                                         \
    {                                                                                              \
        .behavior_dev = DEVICE_DT_NAME(DT_INST_PHANDLE_BY_IDX(n, bindings, idx)),                \
        .param1 = COND_CODE_0(DT_INST_PHA_HAS_CELL_AT_IDX(n, bindings, idx, param1), (0),        \
                               (DT_INST_PHA_BY_IDX(n, bindings, idx, param1))),                   \
        .param2 = COND_CODE_0(DT_INST_PHA_HAS_CELL_AT_IDX(n, bindings, idx, param2), (0),        \
                               (DT_INST_PHA_BY_IDX(n, bindings, idx, param2))),                   \
    }

#define ZIP_ZOOM_INST(n)                                                                           \
    static const struct zip_zoom_config zip_zoom_config_##n = {                                   \
        .event_code       = DT_INST_PROP(n, event_code),                                          \
        .sensitivity      = DT_INST_PROP(n, sensitivity),                                         \
        .track_remainders = DT_INST_PROP(n, track_remainders),                                    \
        .zoom_out         = BINDING_AT(n, 0),                                                     \
        .zoom_in          = BINDING_AT(n, 1),                                                     \
    };                                                                                             \
    static struct zip_zoom_data zip_zoom_data_##n;                                                 \
    DEVICE_DT_INST_DEFINE(n, NULL, NULL, &zip_zoom_data_##n, &zip_zoom_config_##n, POST_KERNEL,   \
                          CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &zoom_driver_api);

DT_INST_FOREACH_STATUS_OKAY(ZIP_ZOOM_INST)
