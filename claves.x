struct args
{
    int key;
    string value1<256>;
    int N_value2;
    double V_value2[32];
};

struct get_value_result
{
    string value1<256>;
    int N_value2;
    double V_value2[32];
    int status;
};

program CLAVES {
    version CLAVESVER {
        int rpc_init(void) = 1;
        int rpc_set_value(struct args arg) =2;
        get_value_result rpc_get_value(int key) = 3;
        int rpc_modify_value(struct args arg)=4;
        int rpc_delete_key(int key) = 5;
        int rpc_exist(int key)=6;
        } = 1;
} = 99;