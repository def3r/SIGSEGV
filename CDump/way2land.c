#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>

// [NOTE:] WAYLAND OBJECTS ARE NOT REAL! THEY'RE A HOAX!
// THEY STORE NOTHING AND HAVE COMPLETELY OPAQUE struct {};
// YOU ARE ONLY SUPPOSED TO USE EM AS ptrs AND NOT CARE
// WHATS INSIDE EM!

// Thankfully there exist some good resources
// https://jan.newmarch.name/Wayland/ProgrammingClient/
// https://wayland-book.com/wayland-display/event-loop.html
// https://bugaevc.gitbooks.io/writing-wayland-clients/content/

// Wayland
// Wayland is object oriented: all coms b/w a client and the server is
// formulated in the form of method calls on some objects.
// These objects are just a handy abstraction and do not actually exist
// anywhere.
// Object methods are fo 2 kinds: requests and events.
// Requests -> by clients
// Events   -> issued by server
// Wayland Protocol works by issuing requests and events that act on objects.
// Each object has an interface which defines what reqs and events are
// possible and signature of each.
// There is no mechanism to return anything from a method call in Wayland.
// This is because, Wayland is msg based. A msg contains the:
//    1. Object ID
//    2. Opcode (statically known ID) of method
//    3. Args
//  (Wire format)
//  (Wayland Comms happen in wire format which makes sure that comms happen
//  over Unix domain stream socket, located in fs @ path:
//  $XDG_RUNTIME_DIR/$WAYLAND_DISPLAY)
// Wayland is async, -> Sending msgs blocks neither the client nor the server.
// Thus, there is no roundtrip delay (time to send signal + time to get
// acknowkledgement that signal was received)
//

// Seats
// It provides an abstraction over input events on Wayland
// So, a seat can be imagined as a literal seat where the user sits and operates
// the computer, and have upto 1 keyboard and 1 ptr dev
//

struct wl_compositor *compositor = NULL;
struct wl_data_device_manageer *devmgr = NULL;

// wl_registry_bind
// Binding is the process of taking a known object and assigning an ID to
// it.
// Once the client binds to a registry, server emits the global event
// several times to advertise which interface it supports
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
  // connects to the wayland display server
  struct wl_display *display = wl_display_connect(NULL);

  if (!display) {
    printf("Display NULL");
    exit(EXIT_FAILURE);
  }
  printf("Connected\n");

  // registry
  // wl_registry -> global singleton
  // Objects required to communicate with the wayland server
  // Each request and event is assoc with an object ID
  // Once the client recieves the msg, we need to know the interface of the
  // object ID
  // To solve this, we bind(...) and object ID, and agree on the interface used
  // for it in future
  // Server offers a list of global objects.
  // These Objects provide the information and functionality
  // After registry object is created, server emits the global event for each
  // global var available on the server
  // This is when we bind to required globals
  struct wl_registry *registry = wl_display_get_registry(display);

  // Event Handlers
  const struct wl_registry_listener regListener = {globalRegHandler,
                                                   globalRegRemover};
  wl_registry_add_listener(registry, &regListener, NULL);

  // blocks the client untill all the pending methods(req & events) are sent and
  // received w/ all event listeners executed.
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

  // Wayland Client Event Loop
  // wl_display_dispatch -> process incoming events
  // wl_display_flush    -> flush outgoing requests
  // Default way
  while (wl_display_dispatch(display) != -1) {
  }
  // One can also create a custom event loop, and get
  // the fd of the Wayland disaply with wl_display_get_fd

  wl_display_disconnect(display);
  printf("Disconnected from display\n");

  exit(EXIT_SUCCESS);
}
