#include "register_types.h"
#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/project_settings.hpp>

#include "PhotonClient.h"
#include "PhotonView.h"
#include "PhotonTransformView.h"
#include "PhotonRigidbodyView.h"
#include "PhotonAnimatorView.h"

using namespace godot;

void register_photon_settings() {
    ProjectSettings *settings = ProjectSettings::get_singleton();
    String app_id_path = "addons/photon/connection/app_id";
    String app_version_path = "addons/photon/connection/app_version";

    if (!settings->has_setting(app_id_path)) settings->set_setting(app_id_path, "");
    settings->set_initial_value(app_id_path, ""); 

    if (!settings->has_setting(app_version_path)) settings->set_setting(app_version_path, "1.0");
    settings->set_initial_value(app_version_path, "1.0");

    settings->set_as_basic(app_id_path, true);
    settings->set_as_basic(app_version_path, true);
}

void initialize_photon_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) return;
    
    ClassDB::register_class<PhotonClient>();
    ClassDB::register_class<PhotonView>();
    ClassDB::register_class<PhotonTransformView>();
    ClassDB::register_class<PhotonRigidbodyView>();
    ClassDB::register_class<PhotonAnimatorView>();
    
    register_photon_settings();
}

void uninitialize_photon_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) return;
}

extern "C" {
GDExtensionBool GDE_EXPORT photon_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
    godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

    init_obj.register_initializer(initialize_photon_module);
    init_obj.register_terminator(uninitialize_photon_module);
    init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

    return init_obj.init();
}
}