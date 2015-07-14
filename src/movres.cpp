#include "../include/wm.hpp"

void Move::initOwnership() {
    owner->name = "move";
    owner->compatAll = true;
}

void Move::init() {
    win = nullptr;
    hook.action = std::bind(std::mem_fn(&Move::Intermediate), this);
    core->addHook(&hook);

    using namespace std::placeholders;

    press.active = true;
    press.type   = BindingTypePress;
    press.mod    = Mod1Mask;
    press.button = Button1;
    press.action = std::bind(std::mem_fn(&Move::Initiate), this, _1);
    core->addBut(&press);

    release.active = false;
    release.type   = BindingTypeRelease;
    release.mod    = AnyModifier;
    release.button = Button1;
    release.action = std::bind(std::mem_fn(&Move::Terminate), this, _1);
    core->addBut(&release);
}

void Move::Initiate(Context *ctx) {
    auto xev = ctx->xev.xbutton;
    auto w = core->getWindowAtPoint(xev.x_root, xev.y_root);

    if(!w)
        return;
    win = w;

    if(!core->activateOwner(owner)){
        return;
    }

    owner->grab();

    core->focusWindow(w);
    hook.enable();
    release.active = true;

    this->sx = xev.x_root;
    this->sy = xev.y_root;
    core->setRedrawEverything(true);
}

void Move::Terminate(Context *ctx) {

    if(!ctx)
        return;

    hook.disable();
    release.active = false;
    core->deactivateOwner(owner);


    auto xev = ctx->xev.xbutton;

    win->transform.translation = glm::mat4();

    int dx = (xev.x_root - sx) * core->scaleX;
    int dy = (xev.y_root - sy) * core->scaleY;

    int nx = win->attrib.x + dx;
    int ny = win->attrib.y + dy;

    WinUtil::moveWindow(win, nx, ny);
    core->setRedrawEverything(false);

    core->focusWindow(win);
    core->damageWindow(win);
    core->redraw = true;
}

void Move::Intermediate() {

    GetTuple(cmx, cmy, core->getMouseCoord());
    GetTuple(w, h, core->getScreenSize());

    win->transform.translation =
        glm::translate(glm::mat4(), glm::vec3(
                    float(cmx - sx) / float(w / 2.0),
                    float(sy - cmy) / float(h / 2.0),
                    0.f));

    core->redraw = true;
}


void Resize::initOwnership() {
    owner->name = "resize";
    owner->compatAll = true;
}

void Resize::init() {
    win = nullptr;

    hook.action = std::bind(std::mem_fn(&Resize::Intermediate), this);
    core->addHook(&hook);

    using namespace std::placeholders;

    press.active = true;
    press.type   = BindingTypePress;
    press.mod    = ControlMask;
    press.button = Button1;
    press.action = std::bind(std::mem_fn(&Resize::Initiate), this, _1);
    core->addBut(&press);


    release.active = false;
    release.type   = BindingTypeRelease;
    release.mod    = AnyModifier;
    release.button = Button1;
    release.action = std::bind(std::mem_fn(&Resize::Terminate), this,_1);
    core->addBut(&release);
}

void Resize::Initiate(Context *ctx) {
    if(!ctx)
        return;

    auto xev = ctx->xev.xbutton;
    auto w = core->getWindowAtPoint(xev.x_root,xev.y_root);

    if(!w)
        return;

    if(!core->activateOwner(owner)) {
        return;
    }

    owner->grab();

    core->focusWindow(w);
    win = w;
    hook.enable();
    release.active = true;

    if(w->attrib.width == 0)
        w->attrib.width = 1;
    if(w->attrib.height == 0)
        w->attrib.height = 1;

    this->sx = xev.x_root;
    this->sy = xev.y_root;

    owner->active = true;
    core->setRedrawEverything(true);
}

void Resize::Terminate(Context *ctx) {
    if(!ctx)
        return;

    hook.disable();
    release.active = false;

    win->transform.scalation = glm::mat4();
    win->transform.translation = glm::mat4();

    GetTuple(cmx, cmy, core->getMouseCoord());

    int dw = (cmx - sx) * core->scaleX;
    int dh = (cmy - sy) * core->scaleY;

    int nw = win->attrib.width  + dw;
    int nh = win->attrib.height + dh;
    WinUtil::resizeWindow(win, nw, nh);

    core->setRedrawEverything(false);
    core->damageWindow(win);
    core->deactivateOwner(owner);
}

void Resize::Intermediate() {


    GetTuple(cmx, cmy, core->getMouseCoord());
    int dw = cmx - sx;
    int dh = cmy - sy;

    int nw = win->attrib.width  + dw;
    int nh = win->attrib.height + dh;

    float kW = float(nw) / float(win->attrib.width );
    float kH = float(nh) / float(win->attrib.height);


    GetTuple(sw, sh, core->getScreenSize());

    float w2 = float(sw) / 2.;
    float h2 = float(sh) / 2.;

    float tlx = float(win->attrib.x) - w2,
          tly = h2 - float(win->attrib.y);

    float ntlx = kW * tlx;
    float ntly = kH * tly;

    win->transform.translation =
    glm::translate(glm::mat4(), glm::vec3(
                float(tlx - ntlx) / w2, float(tly - ntly) / h2,
                0.f));

    win->transform.scalation =
        glm::scale(glm::mat4(), glm::vec3(kW, kH, 1.f));
    core->redraw = true;
}
