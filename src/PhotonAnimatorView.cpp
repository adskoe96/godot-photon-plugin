#include "PhotonAnimatorView.h"
#include "PhotonClient.h"
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector3.hpp>

using namespace godot;

void PhotonAnimatorView::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_anim_tree_path", "path"), &PhotonAnimatorView::set_anim_tree_path);
    ClassDB::bind_method(D_METHOD("get_anim_tree_path"), &PhotonAnimatorView::get_anim_tree_path);
    ClassDB::bind_method(D_METHOD("set_sync_parameters", "params"), &PhotonAnimatorView::set_sync_parameters);
    ClassDB::bind_method(D_METHOD("get_sync_parameters"), &PhotonAnimatorView::get_sync_parameters);
    ClassDB::bind_method(D_METHOD("set_sync_rate", "rate"), &PhotonAnimatorView::set_sync_rate);
    ClassDB::bind_method(D_METHOD("get_sync_rate"), &PhotonAnimatorView::get_sync_rate);
    ClassDB::bind_method(D_METHOD("set_lerp_speed", "speed"), &PhotonAnimatorView::set_lerp_speed);
    ClassDB::bind_method(D_METHOD("get_lerp_speed"), &PhotonAnimatorView::get_lerp_speed);
    ClassDB::bind_method(D_METHOD("receive_sync_data", "data"), &PhotonAnimatorView::receive_sync_data);

    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "anim_tree_path"), "set_anim_tree_path", "get_anim_tree_path");
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "sync_parameters"), "set_sync_parameters", "get_sync_parameters");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "sync_rate"), "set_sync_rate", "get_sync_rate");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "lerp_speed"), "set_lerp_speed", "get_lerp_speed");
}

PhotonAnimatorView::PhotonAnimatorView() {
    view = nullptr;
    anim_tree = nullptr;
    sync_timer = 0.0;
    sync_rate = 15.0;
    lerp_speed = 10.0;
}

PhotonAnimatorView::~PhotonAnimatorView() {}

void PhotonAnimatorView::_notification(int p_what) {
    if (p_what == NOTIFICATION_ENTER_TREE) {
        set_process(true);
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

AnimationTree* PhotonAnimatorView::get_anim_tree() {
    if (anim_tree) return anim_tree;
    if (!anim_tree_path.is_empty()) {
        if (Node* n = get_node_or_null(anim_tree_path)) {
            anim_tree = Object::cast_to<AnimationTree>(n);
        }
    }
    return anim_tree;
}

void PhotonAnimatorView::_process(double delta) {
    if (!view) return;
    AnimationTree* tree = get_anim_tree();
    if (!tree) return;

    if (view->is_mine()) {
        sync_timer += delta;
        if (sync_timer >= (1.0 / sync_rate)) {
            sync_timer = 0.0;
            
            Dictionary data;
            data["vid"] = view->get_view_id();
            Dictionary params_data;
            
            for (int i = 0; i < sync_parameters.size(); i++) {
                String param_name = sync_parameters[i];
                params_data[param_name] = tree->get(param_name);
            }
            
            data["params"] = params_data;
            PhotonClient::get_singleton()->raise_custom_event(PhotonClient::EVENT_ANIMATOR_SYNC, data, PhotonView::RPC_OTHERS, false);
        }
    } else {
        Array keys = network_data.keys();
        for (int i = 0; i < keys.size(); i++) {
            String key = keys[i];
            Variant target_val = network_data[key];
            Variant current_val = tree->get(key);
            
            if (target_val.get_type() == Variant::FLOAT && current_val.get_type() == Variant::FLOAT) {
                float t = (float)target_val;
                float c = (float)current_val;
                tree->set(key, c + (t - c) * (delta * lerp_speed));
            } 
            else if (target_val.get_type() == Variant::VECTOR2 && current_val.get_type() == Variant::VECTOR2) {
                Vector2 t = target_val;
                Vector2 c = current_val;
                tree->set(key, c.lerp(t, delta * lerp_speed));
            } 
            else {
                tree->set(key, target_val);
            }
        }
    }
}

void PhotonAnimatorView::receive_sync_data(const Dictionary& data) {
    network_data = data;
}

void PhotonAnimatorView::set_anim_tree_path(const NodePath& path) { anim_tree_path = path; anim_tree = nullptr; }
NodePath PhotonAnimatorView::get_anim_tree_path() const { return anim_tree_path; }
void PhotonAnimatorView::set_sync_parameters(const Array& params) { sync_parameters = params; }
Array PhotonAnimatorView::get_sync_parameters() const { return sync_parameters; }
void PhotonAnimatorView::set_sync_rate(double rate) { sync_rate = rate; }
double PhotonAnimatorView::get_sync_rate() const { return sync_rate; }
void PhotonAnimatorView::set_lerp_speed(float speed) { lerp_speed = speed; }
float PhotonAnimatorView::get_lerp_speed() const { return lerp_speed; }