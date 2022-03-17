#include <iostream> // std::cout, std::endl
#include <stdio.h> // printf
#include <stdint.h> // uint8_t, uint16_t, ...
#include <math.h> // sinf
#include <immintrin.h> // rdtsc
#include <SDL.h>
#include <windows.h>
#include <winbase.h> //copyfile
#include "game.h"
#include "macros.h"
//#include "game.cpp"
#include "utility.h"
#define SDL_AUDIODRIVER DirectSound

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif
// FILE //
static inline FILETIME windows_file_getFileTime(char* _FileName)
{
	FILETIME _lastTimeGameLibraryCompiled = {};
	WIN32_FIND_DATA _findData;
	HANDLE _findHandle = FindFirstFileA(_FileName, &_findData);
	if(_findHandle != INVALID_HANDLE_VALUE)
	{
		_lastTimeGameLibraryCompiled = _findData.ftLastWriteTime;
		FindClose(_findHandle);
	}
	return _lastTimeGameLibraryCompiled;
}

// PERFORMANCE //
static float windows_performance_secondsElapsed(uint64_t _PreviousCounter, uint64_t _CurrentCounter)
{
	return ((float)(_CurrentCounter - _PreviousCounter) / (float)(SDL_GetPerformanceFrequency()));
}


// RENDER //
static void windows_render_updateWindow(SDL_Window* _Window, SDL_Renderer* _Renderer, SDL_Texture* _Texture, void* _Pixels, int _TextureWidth)
{
	SDL_UpdateTexture(_Texture, 0, _Pixels, _TextureWidth * 4);
	SDL_RenderCopy(_Renderer, _Texture, 0, 0);
	SDL_RenderPresent(_Renderer);
}


// AUDIO //
struct windows_audio_userAudioData
{
	int size;
	int writeCursor;
	int playCursor;
	void* data;
};
static void windows_audio_callback(void* _UserData, uint8_t* _Stream, int _Length)
{
	windows_audio_userAudioData* _userAudioData = (windows_audio_userAudioData*)_UserData;

	int _region1Size = _Length;
	int _region2Size = 0;
	if((_userAudioData->playCursor + _Length) > _userAudioData->size)
	{
		_region1Size = _userAudioData->size - _userAudioData->playCursor;
		_region2Size = _Length - _region1Size;
	}

	memcpy(_Stream, (uint8_t*)_userAudioData->data + _userAudioData->playCursor, _region1Size);
	memcpy(_Stream + _region1Size, _userAudioData->data, _region2Size);
	_userAudioData->playCursor = (_userAudioData->playCursor + _Length) % _userAudioData->size;
	_userAudioData->writeCursor = (_userAudioData->playCursor + _Length) % _userAudioData->size;
}


int main(int argc, char *argv[])
{
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO) < 0)
	{
		//SDL_GetError( )
	}
	SDL_Window* _window = SDL_CreateWindow("game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_RESIZABLE|SDL_WINDOW_OPENGL);
	//SDL_SetWindowFullscreen(_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
	if(_window)
	{
		SDL_Renderer* _renderer = SDL_CreateRenderer(_window, -1, 0);
		if(_renderer)
		{

			// Get Monitor Refresh
			float _gameUpdateHz = 60;
			int _displayIndex = SDL_GetWindowDisplayIndex(_window);
			SDL_DisplayMode _displayMode = {};
			int _displayModeResult = SDL_GetDesktopDisplayMode(_displayIndex, &_displayMode);
			if(_displayModeResult == 0 && _displayMode.refresh_rate > 1)
			{
				_gameUpdateHz = (float)_displayMode.refresh_rate;
			}
			float _targetSecondsPerFrame = 1.0f / _gameUpdateHz;


			// INIT RENDER //
			game_render_buffer _renderBuffer;
			_renderBuffer.memory = 0;
			_renderBuffer.width = 0;
			_renderBuffer.height = 0;
			SDL_GetWindowSize(_window, &_renderBuffer.width, &_renderBuffer.height);
			_renderBuffer.pitch = _renderBuffer.width * game_render_bytesPerPixel;
			SDL_Texture* _texture = 0;
			if(_renderBuffer.memory)
			{
				free(_renderBuffer.memory);
			}
			if(_texture)
			{
				SDL_DestroyTexture(_texture);
			}
			_texture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, _renderBuffer.width, _renderBuffer.height);
			_renderBuffer.memory = malloc(_renderBuffer.width * _renderBuffer.height * game_render_bytesPerPixel);


			// INIT INPUT //
			SDL_Keycode _keybinds[game_input_numberOfButtonbinds] = {}; // TODO: load from file
			_keybinds[game_input_1] = 'q';
			_keybinds[game_input_2] = 'w';
			_keybinds[game_input_3] = 'f';
			_keybinds[game_input_4] = 'p';
			_keybinds[game_input_escape] = SDLK_ESCAPE;
			_keybinds[5] = SDLK_UP;
			_keybinds[6] = SDLK_DOWN;
			_keybinds[7] = SDLK_LEFT;
			_keybinds[8] = SDLK_RIGHT;

			// INIT AUDIO //
			game_sound_buffer _soundBuffer;
			_soundBuffer.samplesPerSecond = 48000;
			int _bytesPerSample = sizeof(int16_t) * 2;
			int16_t _toneVolume = 3000;
			float _tSine = 0.0f;
			int _toneHz = 256;
			int _wavePeriod = _soundBuffer.samplesPerSecond / _toneHz;
			uint32_t _runningSampleIndex = 0;
			uint32_t _secondaryBufferSize = _soundBuffer.samplesPerSecond * _bytesPerSample;
			//uint32_t _safetyBytes = (int)(((float)_soundBuffer.samplesPerSecond * (float)_bytesPerSample / _gameUpdateHz) / 2.0f);
			uint32_t _safetyBytes = _soundBuffer.samplesPerSecond / 15;
			windows_audio_userAudioData _audioUserData = {};
			_audioUserData.size = _soundBuffer.samplesPerSecond * _bytesPerSample;
			_audioUserData.data = calloc(_audioUserData.size, 1);

			_audioUserData.playCursor = 0;
			_audioUserData.writeCursor = 0;
			SDL_AudioSpec _audioSettings = {};
			_audioSettings.freq = _soundBuffer.samplesPerSecond;
			_audioSettings.format = AUDIO_S16LSB;
			_audioSettings.channels = 2;
			_audioSettings.samples = 512;
			_audioSettings.callback = &windows_audio_callback;
			_audioSettings.userdata = &_audioUserData;
			SDL_OpenAudio(&_audioSettings, 0);
			if(_audioSettings.format != AUDIO_S16LSB)
			{
				// failed to get sample format
				SDL_CloseAudio();
			}
			SDL_PauseAudio(0);
			bool _soundIsPlaying = false;
			_soundBuffer.samples = (int16_t*)calloc(_soundBuffer.samplesPerSecond + 8, _bytesPerSample); // 8 is a small buffer
			//_soundBuffer.samples = (int16_t*)VirtualAlloc(0, _secondaryBufferSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

			// INIT MEMORY //
			game_memory_allocated _allocatedMemory;
			_allocatedMemory.permanentStorageSize = macros_amount_megabytes(64);
			_allocatedMemory.transientStorageSize = macros_amount_gigabytes(1);
			void* _baseAddress = 0;
			uint64_t _totalStorageSize = _allocatedMemory.permanentStorageSize + _allocatedMemory.transientStorageSize;
			_allocatedMemory.permanentStorage = VirtualAlloc(_baseAddress, (SIZE_T)_totalStorageSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
			_allocatedMemory.transientStorage = ((uint8_t*)_allocatedMemory.permanentStorage + _allocatedMemory.permanentStorageSize);
			if(!_allocatedMemory.permanentStorage)
			{
				// TODO: log error
				return 1;
			}


			// INIT GAME //
			// load game library functions
			game_input_buffer _inputBuffer;
			game_main_state* _gameState = (game_main_state*)_allocatedMemory.permanentStorage;
			_gameState->secondsPassed = _targetSecondsPerFrame;
			uint8_t _gameIsRunning = true;
			void* _gameLibrary = NULL;
			char* _initFunctionName = "game_main_init";
			char* _updateFunctionName = "game_main_update";
			void (*game_main_init)(uint8_t* _IsRunning, game_sound_buffer* _SoundBuffer, game_render_buffer* _RenderBuffer, game_input_buffer* _InputBuffer, game_memory_allocated* _AllocatedMemory);
			void (*game_main_update)(uint8_t* _IsRunning, game_sound_buffer* _SoundBuffer, game_render_buffer* _RenderBuffer, game_input_buffer* _InputBuffer, game_memory_allocated* _AllocatedMemory);
			CopyFile("game.dll", "game_temp.dll", FALSE);
			_gameLibrary = SDL_LoadObject("game_temp.dll");
			game_main_init = (void (*)(uint8_t*, game_sound_buffer*, game_render_buffer*, game_input_buffer*, game_memory_allocated*))SDL_LoadFunction(_gameLibrary, _initFunctionName);
			game_main_update = (void (*)(uint8_t*, game_sound_buffer*, game_render_buffer*, game_input_buffer*, game_memory_allocated*))SDL_LoadFunction(_gameLibrary, _updateFunctionName);
			game_main_init(&_gameIsRunning, &_soundBuffer, &_renderBuffer, &_inputBuffer, &_allocatedMemory);


			// INIT PERFORMANCE //
			//uint64_t _performanceCounterFrequency = SDL_GetPerformanceFrequency();
			uint64_t _lastPerformanceCounter = SDL_GetPerformanceCounter();
			uint64_t _lastCycleCount = __rdtsc();
			FILETIME _lastTimeGameLibraryCompiled = windows_file_getFileTime("game.dll");
			SDL_Event _event;
			while(_gameIsRunning)
			{
#if 1
				FILETIME _currentTimeGameLibraryCompiled = windows_file_getFileTime("game.dll");
				if(CompareFileTime(&_currentTimeGameLibraryCompiled, &_lastTimeGameLibraryCompiled) != 0)
				{
					SDL_UnloadObject(_gameLibrary);
					CopyFile("game.dll", "game_temp.dll", FALSE);
					_gameLibrary = SDL_LoadObject("game_temp.dll");
					game_main_update = (void (*)(uint8_t*, game_sound_buffer*, game_render_buffer*, game_input_buffer*, game_memory_allocated*))SDL_LoadFunction(_gameLibrary, _updateFunctionName);
				}
#endif
				int _inputBufferButtonIndex = 0;
				while(SDL_PollEvent(&_event))
				{
					switch(_event.type)
					{
						// UPDATE INPUT //
						case SDL_KEYDOWN: // note: keys are only recorded when pressed not when released for this app
							{
								if(_inputBufferButtonIndex < game_input_numberOfButtonsPerFrame)
								{
									uint8_t _keybindIndex = 0;
									while(_keybindIndex < game_input_numberOfActionBinds) 
									{
										if (_event.key.keysym.sym == _keybinds[_keybindIndex])
										{
											_inputBuffer.buttonsPressedDuringFrame[_inputBufferButtonIndex] = _keybindIndex;
											_inputBufferButtonIndex++;
										}
										_keybindIndex++;
									}
									_keybindIndex = 0;
									while(_keybindIndex < game_input_numberOfMovementBinds) 
									{
										uint8_t _keybindOffset = _keybindIndex + game_input_numberOfActionBinds;
										if (_event.key.keysym.sym == _keybinds[_keybindOffset]) // movement keys come after action keys
										{
											_inputBuffer.movementKeyEndedDown[_keybindIndex] = game_input_endedDown;
										}
										_keybindIndex++;
									}
								}
							}
							break;
						case SDL_KEYUP:
							{
									uint8_t _keybindIndex = 0;
									while(_keybindIndex < game_input_numberOfMovementBinds) 
									{
										uint8_t _keybindOffset = _keybindIndex + game_input_numberOfActionBinds;
										if (_event.key.keysym.sym == _keybinds[_keybindOffset]) // movement keys come after action keys
										{
											if(_inputBuffer.movementKeyEndedDown[_keybindIndex] == game_input_endedDown)
											{
												_inputBuffer.movementKeyTransitionCount[_keybindIndex]++;
											}
											_inputBuffer.movementKeyEndedDown[_keybindIndex] = game_input_endedUp;
										}
										_keybindIndex++;
									}
							}
							break;
						case SDL_QUIT:
							{
								printf("SDL_QUIT\n");
								_gameIsRunning = false;
							}
							break;

						case SDL_WINDOWEVENT:
							{
								switch(_event.window.event)
								{
									case SDL_WINDOWEVENT_SIZE_CHANGED:
										{
											_renderBuffer.width = (uint32_t)_event.window.data1;
											_renderBuffer.height = (uint32_t)_event.window.data2;
											_renderBuffer.pitch = _renderBuffer.width * game_render_bytesPerPixel;
											//printf("SDL_WINDOWEVENT_SIZE_CHANGED (%d, %d)\n", _renderBuffer.width, _renderBuffer.height);
											// resize texture
											if(_renderBuffer.memory)
											{
												free(_renderBuffer.memory);
											}
											if(_texture)
											{
												SDL_DestroyTexture(_texture);
											}
											_texture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, _renderBuffer.width, _renderBuffer.height);
											_renderBuffer.memory = malloc(_renderBuffer.width * _renderBuffer.height * game_render_bytesPerPixel);
										}
										break;

									case SDL_WINDOWEVENT_FOCUS_GAINED:
										{
										}
										break;
									case SDL_WINDOWEVENT_MOVED:
										{
											OutputDebugStringA("moved help!");
										}
										break;


									case SDL_WINDOWEVENT_EXPOSED:
										{
											windows_render_updateWindow(_window, _renderer, _texture, _renderBuffer.memory, _renderBuffer.width);
										}
										break;
								};
							}
							break;
					};
				}
				// UPDATE INPUT MOUSE //
				SDL_PumpEvents();
				SDL_GetMouseState(&_inputBuffer.mouseX, &_inputBuffer.mouseY);
				

				// UPDATE AUDIO //
				SDL_LockAudio();
				int _byteToLock = (_runningSampleIndex * _bytesPerSample) % _secondaryBufferSize;
				int _targetCursor = ((_audioUserData.playCursor + (_safetyBytes * _bytesPerSample)) % _secondaryBufferSize);
				int _bytesToWrite = 0;
				if(_byteToLock > _targetCursor)
				{
					_bytesToWrite = (_secondaryBufferSize - _byteToLock);
					_bytesToWrite += _targetCursor;
				}
				else
				{
					_bytesToWrite = _targetCursor - _byteToLock;
				}
				SDL_UnlockAudio();
				_soundBuffer.sampleCount = macros_align_8(_bytesToWrite / _bytesPerSample);
				_bytesToWrite = _soundBuffer.sampleCount * _bytesPerSample;
				// fill sound buffer
				void* _region1 = (uint8_t*)_audioUserData.data + _byteToLock;
				unsigned int _region1Size = (unsigned int)_bytesToWrite;
				if(_region1Size + _byteToLock > _secondaryBufferSize)
				{
					_region1Size = _secondaryBufferSize - _byteToLock;
				}
				void* _region2 = _audioUserData.data;
				int _region2Size = _bytesToWrite - _region1Size;
				int _region1SampleCount = _region1Size / _bytesPerSample;
				int16_t* _destSample = (int16_t*)_region1;
				int16_t* _sourceSample = _soundBuffer.samples;
				int _sampleIndex = 0;
				while(_sampleIndex < _region1SampleCount)
				{
					*_destSample = *_sourceSample;
					_destSample++;
					_sourceSample++;
					*_destSample = *_sourceSample;
					_destSample++;
					_sourceSample++;
					_runningSampleIndex++;
					_sampleIndex++;
				}
				int _region2SampleCount = _region2Size / _bytesPerSample;
				_destSample = (int16_t*)_region2;
				_sampleIndex = 0;
				while(_sampleIndex < _region2SampleCount)
				{
					*_destSample = *_sourceSample;
					_destSample++;
					_sourceSample++;
					*_destSample = *_sourceSample;
					_destSample++;
					_sourceSample++;
					_runningSampleIndex++;
					_sampleIndex++;
				}


				// UPDATE GAME //
				game_main_update(&_gameIsRunning, &_soundBuffer, &_renderBuffer, &_inputBuffer, &_allocatedMemory);


				// UPDATE FILES //
				if(_gameState->platformMessage == game_memory_saveFile)
				{
					char _fileName[32] = {};
					void* _fileContents = "";
					size_t _fileLength = 0;
					// TODO write keybinds.txt from keybind struct
					// TODO remap keybinds menu
					// TODO save file - what should the saves save
					// TODO map file - how to encode
					switch(_gameState->loadFileType)
					{
						case game_file_object: // working on
							utility_char_replaceInto("assets\\object##.obj", _fileName);
							utility_char_replaceIntoWithTwoDigitsAt(_gameState->loadFileIndex, _fileName, 13);
							break;

						case game_file_keybinds:
							utility_char_replaceInto("keybinds.txt", _fileName);
							break;

						case game_file_save:
							utility_char_replaceInto("save\\save##.save", _fileName);
							utility_char_replaceIntoWithTwoDigitsAt(_gameState->loadFileIndex, _fileName, 9);
							_fileContents = _allocatedMemory.permanentStorage; 
							_fileLength = (size_t)_allocatedMemory.permanentStorageSize;
							break;

						case game_file_character:
							utility_char_replaceInto("assets\\character##.character", _fileName);
							utility_char_replaceIntoWithTwoDigitsAt(_gameState->loadFileIndex, _fileName, 17);
							break;

						case game_file_map:
							utility_char_replaceInto("assets\\map##.map", _fileName);
							utility_char_replaceIntoWithTwoDigitsAt(_gameState->loadFileIndex, _fileName, 10);
							break;
					}
					SDL_RWops* _sdlRWops = SDL_RWFromFile(_fileName, "wb");
					if(_sdlRWops != NULL)
					{
						if(SDL_RWwrite(_sdlRWops, _fileContents, 1, _fileLength) != _fileLength)
						{
							printf("Couldn't fully write string\n");
						}
						SDL_RWclose(_sdlRWops);
					}
					_gameState->platformMessage = _gameState->platformMessage | game_memory_savedFile;
					_gameState->platformMessage = _gameState->platformMessage & ~game_memory_saveFile;
				}
				if(_gameState->platformMessage == game_memory_loadFile)
				{
					char _fileName[32] = {};  

					switch(_gameState->loadFileType)
					{
						case game_file_object: // working on
							utility_char_replaceInto("assets\\object##.obj", _fileName);
							utility_char_replaceIntoWithTwoDigitsAt(_gameState->loadFileIndex, _fileName, 13);
							break;

						case game_file_keybinds:
							utility_char_replaceInto("keybinds.txt", _fileName);
							break;

						case game_file_save:
							utility_char_replaceInto("save\\save##.save", _fileName);
							utility_char_replaceIntoWithTwoDigitsAt(_gameState->loadFileIndex, _fileName, 9);
							break;

						case game_file_character:
							utility_char_replaceInto("assets\\character##.character", _fileName);
							utility_char_replaceIntoWithTwoDigitsAt(_gameState->loadFileIndex, _fileName, 17);
							break;

						case game_file_map:
							utility_char_replaceInto("assets\\map##.map", _fileName);
							utility_char_replaceIntoWithTwoDigitsAt(_gameState->loadFileIndex, _fileName, 10);
							break;
					}
					SDL_RWops* _sdlRWops = SDL_RWFromFile(_fileName, "rb");
					if(_sdlRWops != NULL)
					{
						Sint64 _fileSize = SDL_RWsize(_sdlRWops);
						switch(_gameState->loadFileType)
						{
							case game_file_object: // working on
								SDL_RWread(_sdlRWops, _allocatedMemory.transientStorage, (size_t)_allocatedMemory.transientStorageSize, 1);
								break;

							case game_file_keybinds:
								break;

							case game_file_save:
								SDL_RWread(_sdlRWops, _allocatedMemory.permanentStorage, (size_t)_allocatedMemory.permanentStorageSize, 1);
								break;

							case game_file_character:
								break;

							case game_file_map:
								break;
						}
						SDL_RWclose(_sdlRWops);
					}
					_gameState->platformMessage = _gameState->platformMessage | game_memory_loadedFile;
					_gameState->platformMessage = _gameState->platformMessage & ~game_memory_loadFile;
				}


				// UPDATE PERFORMANCE //
				uint64_t _endCycleCount = __rdtsc();
				float _elapsed = windows_performance_secondsElapsed(_lastPerformanceCounter, SDL_GetPerformanceCounter()) ;
				if(_elapsed < _targetSecondsPerFrame)
				{
					float _timeElapsed = windows_performance_secondsElapsed(_lastPerformanceCounter, SDL_GetPerformanceCounter());
					int32_t _timeToSleep = (int32_t)((_gameState->secondsPassed - _targetSecondsPerFrame) * 1000) - 1;
					if(_timeToSleep > 0)
					{
						SDL_Delay((uint32_t)_timeToSleep);
					}
					while(windows_performance_secondsElapsed(_lastPerformanceCounter, SDL_GetPerformanceCounter()) < _targetSecondsPerFrame)
					{
					}
				}
				_gameState->secondsPassed = windows_performance_secondsElapsed(_lastPerformanceCounter, SDL_GetPerformanceCounter());


				// UPDATE WINDOW //
				windows_render_updateWindow(_window, _renderer, _texture, _renderBuffer.memory, _renderBuffer.width);


				// UPDATE PERFORMANCE //
				_lastCycleCount = _endCycleCount;
				_lastPerformanceCounter = SDL_GetPerformanceCounter();
			}
		}
	}
	SDL_Quit();
	return 0;
}
