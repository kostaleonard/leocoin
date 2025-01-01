/**
 * @brief Runs the peer discovery client.
 */

#include "include/peer_discovery_thread.h"
#include "include/return_codes.h"

int main(int argc, char **argv) {
    return_code_t return_code = SUCCESS;
    discover_peers_args_t args = {0};
    // TODO read bootstrap server address from argv
    // TODO read communication interval from argv
    // TODO launch thread
end:
    return return_code;
}
