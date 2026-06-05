#include "PhotonRigidbodyView.h"
#include "PhotonClient.h"
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void PhotonRigidbodyView::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_sync_linear_velocity", "val"), &PhotonRigidbodyView::set_sync_linear_velocity);
    ClassDB::bind_method(D_METHOD("get_sync_linear_velocity"), &PhotonRigidbodyView::get_sync_linear_velocity);
    ClassDB::bind_method(D_METHOD("set_sync_angular_velocity", "val"), &PhotonRigidbodyView::set_sync_angular_velocity);
    ClassDB::bind_method(D_METHOD("get_sync_angular_velocity"), &PhotonRigidbodyView::get_sync_angular_velocity);

    ClassDB::bind_method(D_METHOD("set_sync_rate", "rate"), &PhotonRigidbodyView::set_sync_rate);
    ClassDB::bind_method(D_METHOD("get_sync_rate"), &PhotonRigidbodyView::get_sync_rate);
    ClassDB::bind_method(D_METHOD("set_lerp_speed", "speed"), &PhotonRigidbodyView::set_lerp_speed);
    ClassDB::bind_method(D_METHOD("get_lerp_speed"), &PhotonRigidbodyView::get_lerp_speed);

    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "sync_linear_velocity"), "set_sync_linear_velocity", "get_sync_linear_velocity");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "sync_angular_velocity"), "set_sync_angular_velocity", "get_sync_angular_velocity");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "sync_rate"), "set_sync_rate", "get_sync_rate");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "lerp_speed"), "set_lerp_speed", "get_lerp_speed");
}

PhotonRigidbodyView::PhotonRigidbodyView() {
    view = nullptr;
    target_3d = nullptr;
    target_2d = nullptr;
    
    sync_linear_velocity = true;
    sync_angular_velocity = true;

    sync_timer = 0.0;
    sync_rate = 15.0;
    lerp_speed = 15.0;
}

PhotonRigidbodyView::~PhotonRigidbodyView() {}

void PhotonRigidbodyView::_notification(int p_what) {
    if (p_what == NOTIFICATION_ENTER_TREE) {
        set_process(true);
        set_physics_process(true);
        
        target_3d = Object::cast_to<RigidBody3D>(get_parent());
        target_2d = Object::cast_to<RigidBody2D>(get_parent());
        
        if (Node* parent = get_parent()) {
            for (int i = 0; i < parent->get_child_count(); i++) {
                if (PhotonView* pv = Object::cast_to<PhotonView>(parent->get_child(i))) {
                    view = pv;
                    break;
                }
            }
        }

        if (view && !view->is_mine()) {
            if (target_3d) {
                network_position = target_3d->get_global_position();
                network_rotation = target_3d->get_global_rotation();
            } else if (target_2d) {
                network_position = target_2d->get_global_position();
                network_rotation = target_2d->get_global_rotation();
            }
        }
    }
}

void PhotonRigidbodyView::_process(double delta) {
    if (!view || view->is_mine()) return;

    if (target_3d) {
        if (network_position.get_type() == Variant::VECTOR3)
            target_3d->set_global_position(((Vector3)target_3d->get_global_position()).lerp(network_position, delta * lerp_speed));
        if (network_rotation.get_type() == Variant::VECTOR3)
            target_3d->set_global_rotation(((Vector3)target_3d->get_global_rotation()).lerp(network_rotation, delta * lerp_speed));
    } else if (target_2d) {
        if (network_position.get_type() == Variant::VECTOR2)
            target_2d->set_global_position(((Vector2)target_2d->get_global_position()).lerp(network_position, delta * lerp_speed));
        if (network_rotation.get_type() == Variant::FLOAT) {
            float c = target_2d->get_global_rotation();
            float t = network_rotation;
            target_2d->set_global_rotation(c + (t - c) * (delta * lerp_speed));
        }
    }
}

void PhotonRigidbodyView::_physics_process(double delta) {
    if (!view || (!target_3d && !target_2d)) return;

    if (view->is_mine()) {
        sync_timer += delta;
        if (sync_timer >= (1.0 / sync_rate)) {
            sync_timer = 0.0;
            
            Dictionary data;
            data["vid"] = view->get_view_id();
            
            if (target_3d) {
                data["p"] = target_3d->get_global_position();
                data["r"] = target_3d->get_global_rotation();
                if (sync_linear_velocity) data["lv"] = target_3d->get_linear_velocity();
                if (sync_angular_velocity) data["av"] = target_3d->get_angular_velocity();
            } else if (target_2d) {
                data["p"] = target_2d->get_global_position();
                data["r"] = target_2d->get_global_rotation();
                if (sync_linear_velocity) data["lv"] = target_2d->get_linear_velocity();
                if (sync_angular_velocity) data["av"] = target_2d->get_angular_velocity();
            }

            PhotonClient::get_singleton()->raise_custom_event(PhotonClient::EVENT_RIGIDBODY_SYNC, data, PhotonView::RPC_OTHERS, false);
        }
    } else {
        if (target_3d) {
            if (sync_linear_velocity && network_linear_velocity.get_type() == Variant::VECTOR3)
                target_3d->set_linear_velocity(network_linear_velocity);
            if (sync_angular_velocity && network_angular_velocity.get_type() == Variant::VECTOR3)
                target_3d->set_angular_velocity(network_angular_velocity);
        } else if (target_2d) {
            if (sync_linear_velocity && network_linear_velocity.get_type() == Variant::VECTOR2)
                target_2d->set_linear_velocity(network_linear_velocity);
            if (sync_angular_velocity && network_angular_velocity.get_type() == Variant::FLOAT)
                target_2d->set_angular_velocity(network_angular_velocity);
        }
    }
}

void PhotonRigidbodyView::receive_sync_data(const Variant& pos, const Variant& rot, const Variant& lin_vel, const Variant& ang_vel) {
    if (pos.get_type() != Variant::NIL) network_position = pos;
    if (rot.get_type() != Variant::NIL) network_rotation = rot;
    if (lin_vel.get_type() != Variant::NIL) network_linear_velocity = lin_vel;
    if (ang_vel.get_type() != Variant::NIL) network_angular_velocity = ang_vel;
}

void PhotonRigidbodyView::set_sync_linear_velocity(bool val) { sync_linear_velocity = val; }
bool PhotonRigidbodyView::get_sync_linear_velocity() const { return sync_linear_velocity; }
void PhotonRigidbodyView::set_sync_angular_velocity(bool val) { sync_angular_velocity = val; }
bool PhotonRigidbodyView::get_sync_angular_velocity() const { return sync_angular_velocity; }
void PhotonRigidbodyView::set_sync_rate(double rate) { sync_rate = rate; }
double PhotonRigidbodyView::get_sync_rate() const { return sync_rate; }
void PhotonRigidbodyView::set_lerp_speed(float speed) { lerp_speed = speed; }
float PhotonRigidbodyView::get_lerp_speed() const { return lerp_speed; }