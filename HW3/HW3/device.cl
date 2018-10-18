// Add you device OpenCL code

#define NUM_VEC		4
#define X_DIM		0
#define Y_DIM		1

typedef struct hello_world_msg {
	char character;
	char4 charvec;
	union U{
		float f;
		short s;
		char c;
	}u;
	uint2 vec[NUM_VEC];
}hello_world_msg_t;

__kernel void helloparallelworld (float3 a, float4 b, float8 c, __global hello_world_msg_t *msg )
{
	size_t struct_size = 0;
	int i = get_global_id(X_DIM); // gidx
	int j = get_global_id(Y_DIM); // gidy
	int k = get_local_id(X_DIM); // work-item id x
	int l = get_local_id(Y_DIM); // work-item id y
	int m = get_group_id(X_DIM); // work-group id x
	int n = get_group_id(Y_DIM); // work-group id y
	printf("global_idx: %d, global_idy: %d\n", i, j);
	printf("local_idx: %d, local_idy: %d\n", k, l);
	printf("work_group_idx: %d, work_group_idy: %d\n", m, n);
	printf("float4 elements: %v\n", b);
	printf("float4 elements - reversed: %v", b.s3210);
	printf(""); //TODO: float16???
	printf("structure: %c, %v, %f, %s, %c, %v, %v, %v, %v\n",
			msg->character, msg->charvec, msg->u.f, msg->u.s, msg->u.c, msg->vec[0],msg->vec[1],msg->vec[2],msg->vec[3]);
	printf("structure size: %d\n", sizeof(msg));
	struct_size = sizeof(msg->character) + sizeof(msg->charvec) + sizeof(msg->u.f) + sizeof(msg->u.s) + sizeof(msg->u.c) + sizeof(vec); //TODO Is this right??
	printf("sizeof element sum in struct: %d", struct_size);
	printf("sizeof union: %d", msg->u);
	printf("sizeof u.c: %d", msg->u.c);
}