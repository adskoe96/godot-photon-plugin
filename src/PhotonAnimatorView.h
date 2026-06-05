#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/animation_tree.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/node_path.hpp>

#include "PhotonView.h"

namespace godot {

class PhotonAnimatorView : public Node {
    GDCLASS(PhotonAnimatorView, Node)

private:
    PhotonView* view;
    AnimationTree* anim_tree;
    NodePath anim_tree_path;

    Array sync_parameters;
    Dictionary network_data;

    double sync_timer;
    double sync_rate;
    float lerp_speed;

    AnimationTree* get_anim_tree();

protected:
    static void _bind_methods();
    void _notification(int p_what);

public:
    PhotonAnimatorView();
    ~PhotonAnimatorView();

    void _process(double delta) override;

    void set_anim_tree_path(const NodePath& path);
    NodePath get_anim_tree_path() const;

    void set_sync_parameters(const Array& params);
    Array get_sync_parameters() const;

    void set_sync_rate(double rate);
    double get_sync_rate() const;

    void set_lerp_speed(float speed);
    float get_lerp_speed() const;

    void receive_sync_data(const Dictionary& data);
};

} // namespace godot