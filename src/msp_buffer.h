typedef struct msp_buffer {
	void *start;
	size_t length;
} msp_buffer;

typedef enum {
	SHAPE_POINT,
	SHAPE_LINE,
	SHAPE_RECTANGLE,
	SHAPE_CIRCLE
} msp_shape_type;

typedef struct msp_shape {
        msp_shape_type shape;
	size_t coord_x;
	size_t coord_y;
	size_t size_x;
	size_t size_y;
} msp_shape;


void msp_buffer_init(msp_buffer*,size_t);
void msp_buffer_kill(msp_buffer*);

void msp_buffer_copy(msp_buffer*, msp_buffer*);
void msp_buffer_addto(msp_buffer*, msp_buffer*);

void msp_buffer_addshapes(msp_buffer*, msp_shape*, size_t);

void msp_buffer_yuy2_to_rgb(msp_buffer*, msp_buffer*);
void msp_buffer_yuy2_to_gs8(msp_buffer*, msp_buffer*);

#define CLEAR(x) memset(&(x), 0, sizeof(x))
