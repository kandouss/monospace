typedef struct msp_buffer {
	void *start;
	size_t length;
} msp_buffer;

typedef enum {
	POINT,
	LINE,
	RECTANGLE,
	CIRCLE
} msp_shape;

typedef struct msp_shape_props {
	size_t coord_x;
	size_t coord_y;
	size_t size_x;
	size_t size_y;
} msp_shape_props;


void msp_buffer_init(msp_buffer*,size_t);
void msp_buffer_kill(msp_buffer*);

void msp_buffer_copy(msp_buffer*, msp_buffer*);
void msp_buffer_addto(msp_buffer*, msp_buffer*);

void msp_buffer_yuy2_to_rgb(msp_buffer*, msp_buffer*);
void msp_buffer_yuy2_to_gs8(msp_buffer*, msp_buffer*);

#define CLEAR(x) memset(&(x), 0, sizeof(x))
