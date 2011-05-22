#ifndef _FRAMES_INC
#define _FRAMES_INC

void PositionFrames();
void SetCListVisible(BOOL visible);

void InitFrames();
void DeinitFrames();

// my own alignment enum
typedef enum {AL_TOP, AL_BOTTOM, AL_LEFT, AL_RIGHT} FrameAlign;

struct FrameListNode {
	CLISTFrame frame_data;
	HWND hwndParent;
	int id;
	struct FrameListNode *next;
	LONG origStyle, origExStyle;
	bool moving, hiding, style_override;
	LONG overrideStyle, overrideExStyle;
	char alpha;
	FrameAlign align;
	int width, height; // for frame (frame_data->height is client - which is only used as input from API [addframe, set frame options])

	int off_x, off_y; // offsets from clist location for non-floating mode

	bool sizing, size_move;
};

extern struct FrameListNode *frame_list;

void SaveFrameState(struct FrameListNode *node);
extern CRITICAL_SECTION cs;

#endif
