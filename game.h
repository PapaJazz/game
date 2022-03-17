#ifndef game_h
#define game_h
#include <stdint.h>
#include "math.h"

// INPUT //
// buttonbinds should be initialized on the platform layer
const int game_input_numberOfButtonbinds = 9;
const int game_input_numberOfButtonsPerFrame = 8;
const int game_input_numberOfButtonsPerMove = 4;
const int game_input_frameThreshholdBetweenInputs = 100;
const int game_input_numberOfActionBinds = 5;
const int game_input_numberOfMovementBinds = 4;
const int game_input_endedDown = 1;
const int game_input_endedUp = 0;
enum game_input_buttons : uint8_t
{
	game_input_none = 99,
	game_input_1 = 0,
	game_input_2 = 1,
	game_input_3 = 2,
	game_input_4 = 3,
	game_input_escape = 4,
	game_input_up = 0,
	game_input_down = 1,
	game_input_left = 2,
	game_input_right = 3
};

// TODO: framesSinceLastInput should be in ms instead of frames
typedef struct game_input_buffer
{
	uint8_t framesSinceLastInput; 
	uint8_t moveBufferIndex;
	uint8_t moveBuffer[game_input_numberOfButtonsPerMove]; 
	uint8_t queuedMoveId;
	uint8_t buttonsPressedDuringFrame[game_input_numberOfButtonsPerFrame];
	uint8_t movementKeyEndedDown[game_input_numberOfMovementBinds];
	uint8_t movementKeyTransitionCount[game_input_numberOfMovementBinds];
	int mouseX;
	int mouseY;
} game_input_buffer;


// FILE //
// NOTE: loadFileType decides what type of file to load and the index of which file to load is loadFileIndex in game_memory_buffer
enum game_file_type : uint8_t
{
	game_file_keybinds,
	game_file_save, 
	game_file_character,
	game_file_object,
	game_file_loaded,
	game_file_map 
};
typedef struct game_file_readResult
{
	uint32_t contentsSize;
	void* contents;
} game_file_readResult;
uint32_t game_file_writeEntireFile(char* _FileName, uint32_t _MemorySize, void* _Memory);


// RENDER //
const int game_render_bytesPerPixel = 4;
const int game_render_width = 500;
const int game_render_height = 500;
typedef struct game_render_buffer 
{
	int greenOffset;
	int blueOffset;
	int width;
	int height;
	int pitch;
	void* memory;
} game_render_buffer;
struct game_render_color
{
	float red;
	float blue;
	float green;
};
struct game_render_face
{
	int vertex1;
	int vertex2;
	int vertex3;
};


// MENU //
const int game_menu_numberOfMenuOptions = 2;
struct game_menu_buffer
{
} game_menu_buffer;


// CHARACTER //
const uint8_t game_character_numberOfCharacters = 1;
enum game_characters
{
	game_character_debugger = 0,
	game_character_golem = 1,
	game_character_webber = 2
};
enum game_character_stance
{
	game_stance_debug = 1 << 0,
	game_stance_standing = 1 << 1,
	game_stance_sliding = 1 << 2,
	game_stance_jumpin = 1 << 3,
	game_stance_falling = 1 << 4,
	game_stance_moving = 1 << 5
};
enum game_character_numberOfMoves
{
	game_character_debuggerMoveCount = 2,
	game_character_golemMoveCount = 40,
	game_character_webberMoveCount = 50
};
enum game_character_moveDataAttributes
{
	game_moveAttribute_bounces = 1 << 0,
	game_moveAttribute_pulls = 1 << 0
};
typedef struct game_character_moveData
{
	uint8_t buttons[game_input_numberOfButtonsPerMove];
	uint8_t numberOfFramesToComplete;
	uint8_t requiredStance;  
	uint8_t stance; // stance the character is put in when move is active
	uint32_t attributes;
	// TODO animation id that points to an animation
} game_character_moveData;
typedef struct game_character_data
{
	int8_t numberOfMoves;
	game_character_moveData* moves;
} game_character_data;


// SOUND //
typedef struct game_sound_buffer
{
	int toneHz;
	int samplesPerSecond;
	int sampleCount;
	float tSine;
	int16_t* samples;
} game_sound_buffer;


// MEMORY //
//note: these messages are used to convey to the platform layer that it needs to update/reload something
const uint8_t game_memory_idleMoveIndex = 0;
enum game_memory_platformMessages : uint16_t
{
	game_memory_none = 0,
	game_memory_reloadKeys = 1 << 1,
	game_memory_saveFile = 1 << 2,
	game_memory_loadFile = 1 << 3,
	game_memory_reloadGameLibrary = 1 << 4,
	game_memory_loadedFile = 1 << 5,
	game_memory_savedFile = 1 << 6
};
struct game_memory_allocator
{
	size_t size;
	size_t used;
	uint8_t* base;
};
typedef struct game_memory_allocated 
{
	uint64_t permanentStorageSize;
	uint64_t transientStorageSize;
	void* permanentStorage;
	void* transientStorage;
} game_memory_allocated;

// CAMERA //
enum game_camera_moveDirection
{
	game_camera_moveLeft,
	game_camera_moveRight,
	game_camera_moveForward,
	game_camera_moveBackward
};
enum game_camera_rotateDirection
{
	game_camera_rotateLeft,
	game_camera_rotateRight,
	game_camera_rotateUp,
	game_camera_rotateDown
};

// MAIN //
struct game_object_positionData
{
	math_4D_vector position;
	math_quaternion_struct rotation;
	math_4D_vector scale;
};

typedef struct game_main_state
{
	float playerMomentum;
	math_2D_vector playerPosition;
	math_2D_vector playerDirection;
	uint8_t playerReset;
	uint64_t platformMessage;
	uint8_t loadFileType; // note: used for denoting what type of a file when loading files
	uint8_t loadFileIndex; // note: used for denoting which file selected when loading files
	uint8_t isRunning;
	uint8_t isInMenu;
	uint8_t menuIndex;
	uint8_t currentCharacter;
	uint8_t currentAnimationFrame;
	uint8_t numberOfFramesToCompleteCurrentMove;
	uint8_t currentMoveListIndex; // current move playing
	uint8_t currentStance; // stances change on frame 1 of move else some moves can't buffer on frame 1 of the move
	game_character_data characters[game_character_numberOfCharacters]; // on character change update this
	float secondsPassed;
	game_memory_allocator allocator;
	game_render_face* triangleFaces; 
	uint32_t triangleCount;
	math_4D_vector* objectVerticies;
	uint32_t objectVerticiesCount;
	game_object_positionData objectPositionData;
	game_object_positionData cameraPositionData;
	math_4D_matrix cameraProjectionMatrix; 
	float rotationCounter;
} game_main_state;
extern "C" void game_main_init(uint8_t* _IsRunning, game_sound_buffer* _SoundBuffer, game_render_buffer* _RenderBuffer, game_input_buffer* _InputBuffer, game_memory_allocated* _AllocatedMemory);
extern "C" void game_main_update(uint8_t* _IsRunning, game_sound_buffer* _SoundBuffer, game_render_buffer* _RenderBuffer, game_input_buffer* _InputBuffer, game_memory_allocated* _AllocatedMemory);
#endif
