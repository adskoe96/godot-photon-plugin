#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/rigid_body3d.hpp>
#include <godot_cpp/classes/rigid_body2d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/variant.hpp>
#include "PhotonView.h"

namespace godot {

class PhotonRigidbodyView : public Node {
    GDCLASS(PhotonRigidbodyView, Node)

private:
    PhotonView* view;
    RigidBody3D* target_3d;
    RigidBody2D* target_2d;

    Variant network_position;
    Variant network_rotation;
    Variant network_linear_velocity;
    Variant network_angular_velocity;

    bool sync_linear_velocity;
    bool sync_angular_velocity;

    double sync_timer;
    double sync_rate;
    float lerp_speed;

protected:
    static void _bind_methods();
    void _notification(int p_what);

public:
    PhotonRigidbodyView();
    ~PhotonRigidbodyView();

    void _process(double delta) override;
    void _physics_process(double delta) override;

    void receive_sync_data(const Variant& pos, const Variant& rot, const Variant& lin_vel, const Variant& ang_vel);

    void set_sync_linear_velocity(bool val); bool get_sync_linear_velocity() const;
    void set_sync_angular_velocity(bool val); bool get_sync_angular_velocity() const;

    void set_sync_rate(double rate);
    double get_sync_rate() const;

    void set_lerp_speed(float speed);
    float get_lerp_speed() const;
};

} // namespace godot