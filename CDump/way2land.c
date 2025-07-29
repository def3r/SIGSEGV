#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>

// Thankfully there exists a good resource
// https://jan.newmarch.name/Wayland/ProgrammingClient/

struct wl_compositor *compositor = NULL;
struct wl_data_device_manageer *devmgr = NULL;

void globalRegHandler(void *data, struct wl_registry *registry, uint32_t id,
                      const char *interface, uint32_t version) {
  printf("Registry Event for %s,\tid %d\n", interface, id);
  if (strcmp(interface, "wl_compositor") == 0) {
    compositor =
        wl_registry_bind(registry, id, &wl_compositor_interface, version);
  } else if (strcmp(interface, "wl_data_device_manager") == 0) {
    devmgr = wl_registry_bind(registry, id, &wl_data_device_manager_interface,
                              version);
  }
}

void globalRegRemover(void *data, struct wl_registry *registry, uint32_t id) {
  printf("Registry Losing Event for %d\n", id);
}

int main() {
  struct wl_display *display = wl_display_connect(NULL);

  if (!display) {
    printf("Display NULL");
    exit(EXIT_FAILURE);
  }
  printf("Connected\n");

  struct wl_registry *registry = wl_display_get_registry(display);
  const struct wl_registry_listener regListener = {globalRegHandler,
                                                   globalRegRemover};
  wl_registry_add_listener(registry, &regListener, NULL);

  wl_display_dispatch(display);
  wl_display_roundtrip(display);

  if (!compositor) {
    printf("Cannot find compositor\n");
    exit(EXIT_FAILURE);
  } else {
    printf("Found Compositor\n");
  }

  if (!devmgr) {
    printf("Cannot find Data Device Manager\n");
    exit(EXIT_FAILURE);
  } else {
    printf("Found Data Device Manager\n");
  }

  wl_display_disconnect(display);
  printf("Disconnected from display\n");

  exit(EXIT_SUCCESS);
}
