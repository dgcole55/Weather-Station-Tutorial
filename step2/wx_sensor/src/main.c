
#include <kos.h>
#include <kos_utils.h>
#include <i2c_client.h>

#define I2C_GET_FEATURES    2

enum token_slot_index {
    TOKEN_CLIENT_TRANSPORT = 1,
    TOKEN_I2C
};

int main(int argc, char *argv[]) {
    kos_status_t status; 

    // Get the i2c port name from the arguments
    // We expect two arguments:
    // 0. app name
    // 1. i2c port name 
    kos_assert_eq(argc, 2, "unexpected number of arguments for weather sensor");
    char* app_name = argv[0];
    char* i2c_port = argv[1];

    kos_printf("Initializing %s\n", app_name);

    // Signal that we are done initializing
    kos_app_ready();

    // Setup the message broker
    kos_cap_t receive_cap = kos_cnode_cap(kos_app_cnode(), KOS_ROOT_RECEIVE);
    kos_cap_set_receive(receive_cap);
  
    status = kos_msg_setup();
    kos_assert_created(status, "failed to setup msg server connection");

    kos_printf("Message broker setup.\n");

    // Create a client transport
    kos_cap_t client_cap = kos_cap_reserve();
    kos_token_t token_client_transport;
    kos_msg_client_t msg_client = {0};

    status = kos_msg_client_create(client_cap, TOKEN_CLIENT_TRANSPORT, &msg_client);
    kos_assert_created(status, "failed to create kos_msg_client");

    kos_printf("Client transport setup.\n");

    /*
    *   I2C SERVER
    */
    status = kos_dir_request(i2c_port, TOKEN_I2C, KOS_MSG_FLAG_SEND_TOKEN, NULL);
    while(status != STATUS_OK) {
        kos_printf("Could not connect to %s. Retrying after 1 s.\n", i2c_port);
        kos_delay(1000000);
        status = kos_dir_request(i2c_port, TOKEN_I2C, KOS_MSG_FLAG_SEND_TOKEN, NULL);
    }
    kos_assert_ok(status, "Problem connecting to i2c service");
  
    kos_printf("I2C connection established.\n");

    /*
    *   Talk to the I2C server
    */
    uint32_t features;
    status = kos_i2c_get_features(TOKEN_I2C, &features);
    kos_assert_ok(status, "Problem with I2C features.\n");

    /* 
    *   This should print   Features 0x00000000 
    */
    kos_printf("I2C features:  0x%08x\n", features);  

    kos_tcb_suspend(KOS_ROOT_SLOT_TCB);
    return 0;

}
