#include "PhotonTransformView.h"
#include "PhotonClient.h"
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void PhotonTransformView::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_sync_position", "val"), &PhotonTransformView::set_sync_position);
    ClassDB::bind_method(D_METHOD("get_sync_position"), &PhotonTransformView::get_sync_position);
    ClassDB::bind_method(D_METHOD("set_sync_rotation", "val"), &PhotonTransformView::set_sync_rotation);
    ClassDB::bind_method(D_METHOD("get_sync_rotation"), &PhotonTransformView::get_sync_rotation);
    ClassDB::bind_method(D_METHOD("set_sync_scale", "val"), &PhotonTransformView::set_sync_scale);
    ClassDB::bind_method(D_METHOD("get_sync_scale"), &PhotonTransformView::get_sync_scale);

    ClassDB::bind_method(D_METHOD("set_sync_rate", "rate"), &PhotonTransformView::set_sync_rate);
    ClassDB::bind_method(D_METHOD("get_sync_rate"), &PhotonTransformView::get_sync_rate);
    ClassDB::bind_method(D_METHOD("set_lerp_speed", "speed"), &PhotonTransformView::set_lerp_speed);
    ClassDB::bind_method(D_METHOD("get_lerp_speed"), &PhotonTransformView::get_lerp_speed);

    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "sync_position"), "set_sync_position", "get_sync_position");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "sync_rotation"), "set_sync_rotation", "get_sync_rotation");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "sync_scale"), "set_sync_scale", "get_sync_scale");

    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "sync_rate"), "set_sync_rate", "get_sync_rate");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "lerp_speed"), "set_lerp_speed", "get_lerp_speed");
}

PhotonTransformView::PhotonTransformView() {
    view = nullptr;
    target_3d = nullptr;
    target_2d = nullptr;
    
    sync_position = true;
    sync_rotation = true;
    sync_scale = false;

    sync_timer = 0.0;
    sync_rate = 20.0;
    lerp_speed = 15.0;
}

PhotonTransformView::~PhotonTransformView() {}

void PhotonTransformView::_notification(int p_what) {
    if (p_what == NOTIFICATION_ENTER_TREE) {
        set_process(true);
        set_physics_process(true);

        target_3d = Object::cast_to<Node3D>(get_parent());
        target_2d = Object::cast_to<Node2D>(get_parent());
        
        if (Node* parent = get_parent()) {
            for (int i = 0; i < parent->get_child_count(); i++) {
                if (PhotonView* pv = Object::cast_to<PhotonView>(parent->get_child(i))) {
                    view = pv;
                    break;
                }
            }
        }
    }
}

void PhotonTransformView::_process(double delta) {
    if (!view || view->is_mine()) return;

    if (target_3d) {
        if (sync_position && network_position.get_type() == Variant::VECTOR3)
            target_3d->set_position(((Vector3)target_3d->get_position()).lerp(network_position, delta * lerp_speed));
        
        if (sync_rotation && network_rotation.get_type() == Variant::VECTOR3)
            target_3d->set_rotation(((Vector3)target_3d->get_rotation()).lerp(network_rotation, delta * lerp_speed));
            
        if (sync_scale && network_scale.get_type() == Variant::VECTOR3)
            target_3d->set_scale(((Vector3)target_3d->get_scale()).lerp(network_scale, delta * lerp_speed));
            
    } else if (target_2d) {
        if (sync_position && network_position.get_type() == Variant::VECTOR2)
            target_2d->set_position(((Vector2)target_2d->get_position()).lerp(network_position, delta * lerp_speed));
            
        if (sync_rotation && network_rotation.get_type() == Variant::FLOAT) {
            float c = target_2d->get_rotation();
            float t = network_rotation;
            target_2d->set_rotation(c + (t - c) * (delta * lerp_speed));
        }
            
        if (sync_scale && network_scale.get_type() == Variant::VECTOR2)
            target_2d->set_scale(((Vector2)target_2d->get_scale()).lerp(network_scale, delta * lerp_speed));
    }
}

void PhotonTransformView::_physics_process(double delta) {
    if (!view || (!target_3d && !target_2d)) return;

    if (view->is_mine()) {
        sync_timer += delta;
        if (sync_timer >= (1.0 / sync_rate)) {
            sync_timer = 0.0;
            Dictionary data;
            data["vid"] = view->get_view_id();

            if (target_3d) {
                if (sync_position) data["p"] = target_3d->get_position();
                if (sync_rotation) data["r"] = target_3d->get_rotation();
                if (sync_scale) data["s"] = target_3d->get_scale();
            } else if (target_2d) {
                if (sync_position) data["p"] = target_2d->get_position();
                if (sync_rotation) data["r"] = target_2d->get_rotation();
                if (sync_scale) data["s"] = target_2d->get_scale();
            }

            PhotonClient::get_singleton()->raise_custom_event(PhotonClient::EVENT_TRANSFORM_SYNC, data, PhotonView::RPC_OTHERS, false);
        }
    }
}

void PhotonTransformView::receive_sync_data(const Variant& pos, const Variant& rot, const Variant& scale) {
    if (pos.get_type() != Variant::NIL) network_position = pos;
    if (rot.get_type() != Variant::NIL) network_rotation = rot;
    if (scale.get_type() != Variant::NIL) network_scale = scale;
}

void PhotonTransformView::set_sync_rate(double rate) { sync_rate = rate; }
double PhotonTransformView::get_sync_rate() const { return sync_rate; }

void PhotonTransformView::set_lerp_speed(float speed) { lerp_speed = speed; }
float PhotonTransformView::get_lerp_speed() const { return lerp_speed; }

void PhotonTransformView::set_sync_position(bool val) { sync_position = val; }
bool PhotonTransformView::get_sync_position() const { return sync_position; }

void PhotonTransformView::set_sync_rotation(bool val) { sync_rotation = val; }
bool PhotonTransformView::get_sync_rotation() const { return sync_rotation; }

void PhotonTransformView::set_sync_scale(bool val) { sync_scale = val; }
bool PhotonTransformView::get_sync_scale() const { return sync_scale; }