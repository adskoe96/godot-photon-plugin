#pragma once
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/variant.hpp>

#include "PhotonView.h"

namespace godot {

class PhotonTransformView : public Node {
    GDCLASS(PhotonTransformView, Node)

private:
    PhotonView* view;
    Node3D* target_3d;
    Node2D* target_2d;

    Variant network_position;
    Variant network_rotation;
    Variant network_scale;

    bool sync_position;
    bool sync_rotation;
    bool sync_scale;

    double sync_timer;
    double sync_rate;
    float lerp_speed;

protected:
    static void _bind_methods();
    void _notification(int p_what);

public:
    PhotonTransformView();
    ~PhotonTransformView();

    void _process(double delta) override;
    void _physics_process(double delta) override;

    void receive_sync_data(const Variant& pos, const Variant& rot, const Variant& scale);

    void set_sync_position(bool val); bool get_sync_position() const;
    void set_sync_rotation(bool val); bool get_sync_rotation() const;
    void set_sync_scale(bool val); bool get_sync_scale() const;

    void set_sync_rate(double rate); double get_sync_rate() const;
    void set_lerp_speed(float speed); float get_lerp_speed() const;
};

} // namespace godot