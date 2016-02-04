#include "../include/core.hpp"
#include "../include/opengl.hpp"

#include <GL/glext.h>
#include <EGL/egl.h>
#include <stdio.h>

extern "C" {
#include <wlc/wlc.h>
#include <wlc/wlc-render.h>
}

bool keyboard_key(wlc_handle view, uint32_t time,
        const struct wlc_modifiers *modifiers, uint32_t key,
        enum wlc_key_state state) {

    (void)time, (void)key;
    uint32_t sym = wlc_keyboard_get_keysym_for_key(key, NULL);

    bool grabbed = core->process_key_event(sym, modifiers->mods, state);

    if(grabbed) return true;
    auto w = core->find_window(view);
    if(w && !w->is_visible()) return true;

    return false;
}

bool pointer_button(wlc_handle view, uint32_t time,
        const struct wlc_modifiers *modifiers, uint32_t button,
        enum wlc_button_state state, const struct wlc_point *position) {

    (void)button, (void)time, (void)modifiers;

    assert(core);
    bool grabbed = core->process_button_event(button, modifiers->mods,
            state, *position);

    if(grabbed) return true;

    auto w = core->find_window(view);
    if(w && !w->is_visible()) return true;

    return false;
}

bool pointer_motion(wlc_handle handle, uint32_t time, const struct wlc_point *position) {
    assert(core);
    wlc_pointer_set_position(position);
    bool grabbed = core->process_pointer_motion_event(*position);

    if(grabbed) return true;
    auto w = core->find_window(handle);
    if(w && !w->is_visible()) return true;

    return false;
}

    static void
cb_log(enum wlc_log_type type, const char *str)
{
    (void)type;
    printf("%s\n", str);
}

bool view_created(wlc_handle view) {
    assert(core);
    core->add_window(view);
    return true;
}

void view_destroyed(wlc_handle view) {
    assert(core);
    core->remove_window(core->find_window(view));
    core->focus_window(core->get_active_window());
}

void view_focus(wlc_handle view, bool focus) {
    wlc_view_set_state(view, WLC_BIT_ACTIVATED, focus);
    wlc_view_bring_to_front(view);
}

matrix4x4_t get_transform(glm::mat4 mat) {
    return {{
        mat[0][0], mat[0][1], mat[0][2], mat[0][3],
            mat[1][0], mat[1][1], mat[1][2], mat[1][3],
            mat[2][0], mat[2][1], mat[2][2], mat[2][3],
            mat[3][0], mat[3][1], mat[3][2], mat[3][3],
    }};
}

matrix4x4_t get_mvp_for_view(wlc_handle view) {
    auto w = core->find_window(view);

    if(w) return get_transform(w->transform.compose());
    else  return get_transform(glm::mat4());
}

void view_request_resize(wlc_handle view, uint32_t edges, const struct wlc_point *origin) {
    SignalListenerData data;

    auto w = core->find_window(view);
    if(!w) return;

    data.push_back((void*)(&w));
    data.push_back((void*)(origin));

    core->triggerSignal("resize-request", data);
}


void view_request_move(wlc_handle view, const struct wlc_point *origin) {
    SignalListenerData data;

    auto w = core->find_window(view);
    if(!w) return;

    data.push_back((void*)(&w));
    data.push_back((void*)(origin));

    core->triggerSignal("move-request", data);
}

void output_pre_paint(wlc_handle output) {
    assert(core);

    core->run_hooks();
}

void output_post_paint(wlc_handle output) {
    assert(core);
    if(core->should_redraw())
        wlc_output_schedule_render(output);
}

bool should_repaint_everything() {
    return core->should_repaint_everything();
}

uint32_t get_background_texture() {
    return core->get_background();
}

bool output_created(wlc_handle output) {
    wlc_output_set_mask(output, 1);
    return true;
}

int main(int argc, char *argv[]) {
    static struct wlc_interface interface;

    interface.view.created        = view_created;
    interface.view.destroyed      = view_destroyed;
    interface.view.focus          = view_focus;
    interface.view.render.transform = get_mvp_for_view;

    interface.view.request.resize = view_request_resize;
    interface.view.request.move   = view_request_move;

    interface.output.created = output_created;
    interface.output.render.pre = output_pre_paint;
    interface.output.render.post = output_post_paint;
    interface.output.render.background = get_background_texture;

    interface.keyboard.key = keyboard_key;

    interface.pointer.button = pointer_button;
    interface.pointer.motion = pointer_motion;

    if (!wlc_init(&interface, argc, argv))
        return EXIT_FAILURE;

    core = new Core(0, 0);
    core->init();

    wlc_run();
    return EXIT_SUCCESS;
}
