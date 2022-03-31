// Style:
// Only main functions can call other functions
// Parameters are Capitalized

#include "game.h"
#include "math.h"
#include <math.h> // sinf
#include <stdint.h>
#include <stdlib.h>
#include "utility.h"
#include <float.h> // FLT_MAX
// INPUT //
static void game_input_clearMoveBuffer(uint8_t _MoveBuffer[game_input_numberOfButtonsPerMove])
{
	uint8_t _buttonIndex = 0;
	while(_buttonIndex < game_input_numberOfButtonsPerMove)
	{
		_MoveBuffer[_buttonIndex] = game_input_none;
		_buttonIndex++;
	}
}
static void game_input_clearButtons(uint8_t Button[game_input_numberOfButtonsPerFrame])
{
	uint8_t _buttonIndex = 0;
	while(_buttonIndex < game_input_numberOfButtonsPerFrame)
	{
		Button[_buttonIndex] = game_input_none;
		_buttonIndex++;
	}
}


// RENDER //

static void game_render_rectangle(game_render_buffer* _RenderBuffer, float _MinX, float _MinY, float _MaxX, float _MaxY, math_4D_vector _Color)
{
	int32_t _minX = utility_round_floatToInt32(_MinX);
	int32_t _minY = utility_round_floatToInt32(_MinY);
	int32_t _maxX = utility_round_floatToInt32(_MaxX);
	int32_t _maxY = utility_round_floatToInt32(_MaxY);
	if(_minX < 0)
	{
		_minX = 0;
	}
	if(_minY < 0)
	{
		_minY = 0;
	}
	if(_maxX > _RenderBuffer->width)
	{
		_maxX = _RenderBuffer->width;
	}
	if(_maxY > _RenderBuffer->height)
	{
		_maxY = _RenderBuffer->height;
	}
	uint32_t _color = (utility_round_floatToUint32(_Color.x * 255.0f) << 16) | (utility_round_floatToUint32(_Color.y * 255.0f) << 8) | (utility_round_floatToUint32(_Color.z * 255.0f) << 0); 
	uint8_t* _row = (uint8_t*)(_RenderBuffer->memory) + (_minX * game_render_bytesPerPixel) + (_minY * _RenderBuffer->pitch);    
	int _y = _minY;
	while(_y < _maxY)
	{
		int _x = _minX;
		uint32_t* _pixel = (uint32_t*)_row;
		while(_x < _maxX)
		{
			*_pixel = _color;
			_pixel++;
			_x++;
		}
		_row += _RenderBuffer->pitch;
		_y++;
	}
}



// NOTE: this is an explicit rasteriser rather than an implicit surface rasteriser
static void game_render_triangle(game_render_buffer* _RenderBuffer, game_render_vertex _Vertex1, game_render_vertex _Vertex2, game_render_vertex _Vertex3, game_render_texture _Texture)
{
	// Sort points
	math_4D_vector _minYVertex = _Vertex1.position;
	math_4D_vector _midYVertex = _Vertex2.position;
	math_4D_vector _maxYVertex = _Vertex3.position;
	math_2D_vector _minYVertTextureCoordinates = _Vertex1.textureCoordinates;
	math_2D_vector _midYVertTextureCoordinates = _Vertex2.textureCoordinates;
	math_2D_vector _maxYVertTextureCoordinates = _Vertex3.textureCoordinates;
	if(_maxYVertex.y < _midYVertex.y)
	{
		math_4D_vector _temp = _maxYVertex;
		_maxYVertex = _midYVertex;
		_midYVertex = _temp;
		math_2D_vector _temp2D = _midYVertTextureCoordinates;
		_midYVertTextureCoordinates = _maxYVertTextureCoordinates;
		_maxYVertTextureCoordinates = _temp2D;
	}
	if(_midYVertex.y < _minYVertex.y)
	{
		math_4D_vector _temp = _midYVertex;
		_midYVertex = _minYVertex;
		_minYVertex = _temp;
		math_2D_vector _temp2D = _midYVertTextureCoordinates;
		_midYVertTextureCoordinates =	_minYVertTextureCoordinates ;
		_minYVertTextureCoordinates = _temp2D;
	}
	if(_maxYVertex.y < _midYVertex.y)
	{
		math_4D_vector _temp = _maxYVertex;
		_maxYVertex = _midYVertex;
		_midYVertex = _temp;
		math_2D_vector _temp2D = _midYVertTextureCoordinates;
		_midYVertTextureCoordinates = _maxYVertTextureCoordinates;
		_maxYVertTextureCoordinates = _temp2D;
	}

	// Gradients for calculating Texture Coordinates
	float _oneOverDX = 1.0f / (((_midYVertex.x - _maxYVertex.x) * (_minYVertex.y - _maxYVertex.y)) - ((_minYVertex.x - _maxYVertex.x) * (_midYVertex.y - _maxYVertex.y)));
	float _oneOverDY = -_oneOverDX;
	float _oneOverZ[3];
	_oneOverZ[0] = 1.0f/_minYVertex.w;
	_oneOverZ[1] = 1.0f/_midYVertex.w;
	_oneOverZ[2] = 1.0f/_maxYVertex.w;


	float _texCoordinatesX[3];
	_texCoordinatesX[0] = _minYVertTextureCoordinates.x * _oneOverZ[0];
	_texCoordinatesX[1] = _midYVertTextureCoordinates.x * _oneOverZ[1];
	_texCoordinatesX[2] = _maxYVertTextureCoordinates.x * _oneOverZ[2];

	float _texCoordinatesY[3];
	_texCoordinatesY[0] = _minYVertTextureCoordinates.y * _oneOverZ[0];
	_texCoordinatesY[1] = _midYVertTextureCoordinates.y * _oneOverZ[1];
	_texCoordinatesY[2] = _maxYVertTextureCoordinates.y * _oneOverZ[2];

	float _depth[3];
	_depth[0] = _minYVertex.z;
	_depth[1] = _midYVertex.z;
	_depth[2] = _maxYVertex.z;

	float _texCoordinatesXXStep = (((_texCoordinatesX[1] - _texCoordinatesX[2]) * (_minYVertex.y - _maxYVertex.y)) - ((_texCoordinatesX[0] - _texCoordinatesX[2]) * (_midYVertex.y - _maxYVertex.y))) * _oneOverDX;
	float _texCoordinatesXYStep = (((_texCoordinatesX[1] - _texCoordinatesX[2]) * (_minYVertex.x - _maxYVertex.x)) - ((_texCoordinatesX[0] - _texCoordinatesX[2]) * (_midYVertex.x - _maxYVertex.x))) * _oneOverDY;
	float _texCoordinatesYXStep = (((_texCoordinatesY[1] - _texCoordinatesY[2]) * (_minYVertex.y - _maxYVertex.y)) - ((_texCoordinatesY[0] - _texCoordinatesY[2]) * (_midYVertex.y - _maxYVertex.y))) * _oneOverDX;
	float _texCoordinatesYYStep = (((_texCoordinatesY[1] - _texCoordinatesY[2]) * (_minYVertex.x - _maxYVertex.x)) - ((_texCoordinatesY[0] - _texCoordinatesY[2]) * (_midYVertex.x - _maxYVertex.x))) * _oneOverDY;
	float _depthXStep = (((_depth[1] - _depth[2]) * (_minYVertex.y - _maxYVertex.y)) - ((_depth[0] - _depth[2]) * (_midYVertex.y - _maxYVertex.y))) * _oneOverDX;
	float _depthYStep = (((_depth[1] - _depth[2]) * (_minYVertex.x - _maxYVertex.x)) - ((_depth[0] - _depth[2]) * (_midYVertex.x - _maxYVertex.x))) * _oneOverDY;
	float _oneOverZXStep = (((_oneOverZ[1] - _oneOverZ[2]) * (_minYVertex.y - _maxYVertex.y)) - ((_oneOverZ[0] - _oneOverZ[2]) * (_midYVertex.y - _maxYVertex.y))) * _oneOverDX;
	float _oneOverZYStep = (((_oneOverZ[1] - _oneOverZ[2]) * (_minYVertex.x - _maxYVertex.x)) - ((_oneOverZ[0] - _oneOverZ[2]) * (_midYVertex.x - _maxYVertex.x))) * _oneOverDY;


	// Handedness
	float _x1 = _maxYVertex.x - _minYVertex.x;
	float _y1 = _maxYVertex.y - _minYVertex.y;
	float _x2 = _midYVertex.x - _minYVertex.x;
	float _y2 = _midYVertex.y - _minYVertex.y;
	int _handedness = ((_x1 * _y2) - (_x2 * _y1) >= 0);


	// Temp values used to calculate edge variables
	float _changeInY;
	float _changeInX;
	float _preStepY;
	float _preStepX;


	// Edge 1
	int _topToBottomYStart = (int)ceil(_minYVertex.y);
	int _topToBottomYEnd = (int)ceil(_maxYVertex.y);
	_changeInY = _maxYVertex.y - _minYVertex.y;
	_changeInX = _maxYVertex.x - _minYVertex.x;
	float _topToBottomXStep = (float)_changeInX / (float)_changeInY;
	_preStepY = _topToBottomYStart - _minYVertex.y;
	float _topToBottomCurrentX = _minYVertex.x + _preStepY * _topToBottomXStep;
	_preStepX = _topToBottomCurrentX - _minYVertex.x;
	float _topToBottomTexCoordX = _texCoordinatesX[0] + _texCoordinatesXXStep * _preStepX + _texCoordinatesXYStep * _preStepY;
	float _topToBottomTexCoordXStep = _texCoordinatesXYStep + _texCoordinatesXXStep * _topToBottomXStep;
	float _topToBottomTexCoordY = _texCoordinatesY[0] + _texCoordinatesYXStep * _preStepX + _texCoordinatesYYStep * _preStepY;
	float _topToBottomTexCoordYStep = _texCoordinatesYYStep + _texCoordinatesYXStep * _topToBottomXStep;
	float _topToBottomOneOverZ = _oneOverZ[0] + _oneOverZXStep * _preStepX + _oneOverZYStep * _preStepY;
	float _topToBottomOneOverZStep = _oneOverZYStep + _oneOverZXStep * _topToBottomXStep;
	float _topToBottomDepth = _depth[0] + _depthXStep * _preStepX + _depthYStep * _preStepY;
	float _topToBottomDepthStep = _depthYStep + _depthXStep * _topToBottomXStep;


	// Edge 2
	int _topToMiddleYStart = (int)ceil(_minYVertex.y);
	int _topToMiddleYEnd = (int)ceil(_midYVertex.y);
	_changeInY = _midYVertex.y - _minYVertex.y;
	_changeInX = _midYVertex.x - _minYVertex.x;
	float _topToMiddleXStep = (float)_changeInX / (float)_changeInY;
	_preStepY = _topToMiddleYStart - _minYVertex.y;
	float _topToMiddleCurrentX = _minYVertex.x + _preStepY * _topToMiddleXStep;
	_preStepX = _topToMiddleCurrentX - _minYVertex.x;
	float _topToMiddleTexCoordX = _texCoordinatesX[0] + _texCoordinatesXXStep * _preStepX + _texCoordinatesXYStep * _preStepY;
	float _topToMiddleTexCoordXStep = _texCoordinatesXYStep + _texCoordinatesXXStep * _topToMiddleXStep;
	float _topToMiddleTexCoordY = _texCoordinatesY[0] + _texCoordinatesYXStep * _preStepX + _texCoordinatesYYStep * _preStepY;
	float _topToMiddleTexCoordYStep = _texCoordinatesYYStep + _texCoordinatesYXStep * _topToMiddleXStep;
	float _topToMiddleOneOverZ = _oneOverZ[0] + _oneOverZXStep * _preStepX + _oneOverZYStep * _preStepY;
	float _topToMiddleOneOverZStep = _oneOverZYStep + _oneOverZXStep * _topToMiddleXStep;
	float _topToMiddleDepth = _depth[0] + _depthXStep * _preStepX + _depthYStep * _preStepY;
	float _topToMiddleDepthStep = _depthYStep + _depthXStep * _topToMiddleXStep;


	// Edge 3
	int _middleToBottomYStart = (int)ceil(_midYVertex.y);
	int _middleToBottomYEnd = (int)ceil(_maxYVertex.y);
	_changeInY = _maxYVertex.y - _midYVertex.y;
	_changeInX = _maxYVertex.x - _midYVertex.x;
	float _middleToBottomXStep = (float)_changeInX / (float)_changeInY;
	_preStepY = _middleToBottomYStart - _midYVertex.y;
	float _middleToBottomCurrentX = _midYVertex.x + _preStepY * _middleToBottomXStep;
	_preStepX = _middleToBottomCurrentX - _midYVertex.x;
	float _middleToBottomTexCoordX = _texCoordinatesX[1] + _texCoordinatesXXStep * _preStepX + _texCoordinatesXYStep * _preStepY;
	float _middleToBottomTexCoordXStep = _texCoordinatesXYStep + _texCoordinatesXXStep * _middleToBottomXStep;
	float _middleToBottomTexCoordY = _texCoordinatesY[1] + _texCoordinatesYXStep * _preStepX + _texCoordinatesYYStep * _preStepY;
	float _middleToBottomTexCoordYStep = _texCoordinatesYYStep + _texCoordinatesYXStep * _middleToBottomXStep;
	float _middleToBottomOneOverZ = _oneOverZ[1] + _oneOverZXStep * _preStepX + _oneOverZYStep * _preStepY;
	float _middleToBottomOneOverZStep = _oneOverZYStep + _oneOverZXStep * _middleToBottomXStep;
	float _middleToBottomDepth = _depth[1] + _depthXStep * _preStepX + _depthYStep * _preStepY;
	float _middleToBottomDepthStep = _depthYStep + _depthXStep * _middleToBottomXStep;


	// Top of the triangle
	// triangle 
	int* _leftYStart = &_topToBottomYStart;
	int* _leftYEnd = &_topToBottomYEnd;
	float* _leftXStep = &_topToBottomXStep;
	float* _leftCurrentX = &_topToBottomCurrentX;
	float* _leftDepth = &_topToBottomDepth;
	float* _leftDepthStep = &_topToBottomDepthStep;

	int* _rightYStart = &_topToMiddleYStart;
	int* _rightYEnd = &_topToMiddleYEnd;
	float* _rightXStep = &_topToMiddleXStep;
	float* _rightCurrentX = &_topToMiddleCurrentX;
	float* _rightDepth = &_topToMiddleDepth;
	float* _rightDepthStep = &_topToMiddleDepthStep;

	// Texture 
	float* _leftTexCoordX = &_topToBottomTexCoordX;
	float* _leftTexCoordXStep = &_topToBottomTexCoordXStep;
	float* _leftTexCoordY = &_topToBottomTexCoordY;
	float* _leftTexCoordYStep = &_topToBottomTexCoordYStep;
	float* _leftOneOverZ = &_topToBottomOneOverZ;
	float* _leftOneOverZStep = &_topToBottomOneOverZStep;

	float* _rightTexCoordX = &_topToMiddleTexCoordX;
	float* _rightTexCoordXStep = &_topToMiddleTexCoordXStep;
	float* _rightTexCoordY = &_topToMiddleTexCoordY;
	float* _rightTexCoordYStep = &_topToMiddleTexCoordYStep;
	float* _rightOneOverZ = &_topToMiddleOneOverZ;
	float* _rightOneOverZStep = &_topToMiddleOneOverZStep;

	if(_handedness)
	{
		int* _swapInt;
		_swapInt = _leftYStart;
		_leftYStart = _rightYStart;
		_rightYStart = _swapInt;

		_swapInt = _leftYEnd;
		_leftYEnd = _rightYEnd;
		_rightYEnd = _swapInt;

		float* _swapFloat;
		_swapFloat = _leftXStep;
		_leftXStep = _rightXStep;
		_rightXStep = _swapFloat;

		_swapFloat = _leftCurrentX;
		_leftCurrentX = _rightCurrentX;
		_rightCurrentX = _swapFloat;

		_swapFloat = _leftTexCoordX;
		_leftTexCoordX = _rightTexCoordX;
		_rightTexCoordX = _swapFloat;

		_swapFloat = _leftTexCoordXStep;
		_leftTexCoordXStep = _rightTexCoordXStep;
		_rightTexCoordXStep = _swapFloat;

		_swapFloat = _leftTexCoordY;
		_leftTexCoordY = _rightTexCoordY;
		_rightTexCoordY = _swapFloat;
		
		_swapFloat = _leftTexCoordYStep;
		_leftTexCoordYStep = _rightTexCoordYStep;
		_rightTexCoordYStep = _swapFloat;

		_swapFloat = _leftOneOverZ;
		_leftOneOverZ = _rightOneOverZ;
		_rightOneOverZ = _swapFloat;

		_swapFloat = _leftOneOverZStep;
		_leftOneOverZStep = _rightOneOverZStep;
		_rightOneOverZStep = _swapFloat;

		_swapFloat = _leftDepth;
		_leftDepth = _rightDepth;
		_rightDepth = _swapFloat;

		_swapFloat = _leftDepthStep;
		_leftDepthStep = _rightDepthStep;
		_rightDepthStep = _swapFloat;
	}


	int _yStart = _topToMiddleYStart;
	int _yEnd = _topToMiddleYEnd;
	int _yCurrent = _yStart;
	while(_yCurrent < _yEnd)
	{

		int _xMin = (int)ceil(*_leftCurrentX);
		int _xMax = (int)ceil(*_rightCurrentX);
		float _xPrestep = _xMin - *_leftCurrentX;

		float _xDistance = *_rightCurrentX - *_leftCurrentX;
		float _texCoordXXStep = (*_rightTexCoordX - *_leftTexCoordX) / _xDistance;
		float _texCoordYXStep = (*_rightTexCoordY - *_leftTexCoordY) / _xDistance;
		float _currentOneOverZXStep = (*_rightOneOverZ - *_leftOneOverZ) / _xDistance;
		float _currentDepthStep = (_rightDepthStep - _leftDepthStep) / _xDistance;

		float _texCoordX = *_leftTexCoordX + _texCoordXXStep * _xPrestep;
		float _texCoordY = *_leftTexCoordY + _texCoordYXStep * _xPrestep;
		float _currentOneOverZ = *_leftOneOverZ + _currentOneOverZXStep * _xPrestep;
		float _currentDepth = *_leftDepth + _currentDepthStep * _xPrestep;

		int _xCurrent = _xMin;
		uint8_t* _row = (uint8_t*)(_RenderBuffer->memory) + ((uint32_t)_xCurrent * game_render_bytesPerPixel) + ((_RenderBuffer->height - (uint32_t)_yCurrent) * _RenderBuffer->pitch);    
		uint32_t* _pixel = (uint32_t*)_row;
		while(_xCurrent < _xMax)
		{
			int _depthIndex = _yCurrent * _RenderBuffer->width + _xCurrent;
			if(_currentDepth < _RenderBuffer->depthBuffer[_depthIndex]) // if what you're drawing is closer to your camera draw it
			{
				_RenderBuffer->depthBuffer[_depthIndex] = _currentDepth;
				float _z = 1.0f / _currentOneOverZ;
				int _texX = (int)((_texCoordX * _z) * (float)(_Texture.width - 1) + 0.5f);
				int _texY = (int)((_texCoordY * _z) * (float)(_Texture.height - 1) + 0.5f);
				int _colorIndex = _texY * _Texture.width + _texX;
				math_4D_vector _tempColor = _Texture.color[_colorIndex];
				uint32_t _color = (utility_round_floatToUint32(_tempColor.x * 255.0f) << 16) | (utility_round_floatToUint32(_tempColor.y * 255.0f) << 8) | (utility_round_floatToUint32(_tempColor.z * 255.0f) << 0); 
				*_pixel = _color;
			}
			_pixel++;
			_xCurrent++;
			_currentOneOverZ += _currentOneOverZXStep;
			_texCoordX += _texCoordXXStep;
			_texCoordY += _texCoordYXStep;
			_currentDepth += _currentDepthStep;
		}
		*_leftCurrentX += *_leftXStep;
		*_rightCurrentX += *_rightXStep;
		*_leftTexCoordX += *_leftTexCoordXStep;
		*_leftTexCoordY += *_leftTexCoordYStep;
		*_leftOneOverZ += *_leftOneOverZStep;
		*_rightTexCoordX += *_rightTexCoordXStep;
		*_rightTexCoordY += *_rightTexCoordYStep;
		*_rightOneOverZ += *_rightOneOverZStep;
		*_leftDepth += *_leftDepthStep;
		*_rightDepth += *_rightDepthStep;
		_yCurrent++;
	}


	// Bottom of the triangle 
	// Triangle
	_leftYStart = &_topToBottomYStart;
	_leftYEnd = &_topToBottomYEnd;
	_leftXStep = &_topToBottomXStep;
	_leftCurrentX = &_topToBottomCurrentX;
	_leftDepth = &_topToBottomDepth;
	_leftDepthStep = &_topToBottomDepthStep;

	_rightYStart = &_middleToBottomYStart;
	_rightYEnd = &_middleToBottomYEnd;
	_rightXStep = &_middleToBottomXStep;
	_rightCurrentX = &_middleToBottomCurrentX;
	_rightDepth = &_middleToBottomDepth;
	_rightDepthStep = &_middleToBottomDepthStep;
	 
	// Texture
	_leftTexCoordX = &_topToBottomTexCoordX;
	_leftTexCoordXStep = &_topToBottomTexCoordXStep;
	_leftTexCoordY = &_topToBottomTexCoordY;
	_leftTexCoordYStep = &_topToBottomTexCoordYStep;
	_leftOneOverZ = &_topToBottomOneOverZ;
	_leftOneOverZStep = &_topToBottomOneOverZStep;

	_rightTexCoordX = &_middleToBottomTexCoordX;
	_rightTexCoordXStep = &_middleToBottomTexCoordXStep;
	_rightTexCoordY = &_middleToBottomTexCoordY;
	_rightTexCoordYStep = &_middleToBottomTexCoordYStep;
	_rightOneOverZ = &_middleToBottomOneOverZ;
	_rightOneOverZStep = &_middleToBottomOneOverZStep;

	if(_handedness)
	{
		int* _swapInt;
		_swapInt = _leftYStart;
		_leftYStart = _rightYStart;
		_rightYStart = _swapInt;
		_swapInt = _leftYEnd;
		_leftYEnd = _rightYEnd;
		_rightYEnd = _swapInt;

		float* _swapFloat;
		_swapFloat = _leftXStep;
		_leftXStep = _rightXStep;
		_rightXStep = _swapFloat;
		_swapFloat = _leftCurrentX;
		_leftCurrentX = _rightCurrentX;
		_rightCurrentX = _swapFloat;

		_swapFloat = _leftTexCoordX;
		_leftTexCoordX = _rightTexCoordX;
		_rightTexCoordX = _swapFloat;

		_swapFloat = _leftTexCoordXStep;
		_leftTexCoordXStep = _rightTexCoordXStep;
		_rightTexCoordXStep = _swapFloat;

		_swapFloat = _leftTexCoordY;
		_leftTexCoordY = _rightTexCoordY;
		_rightTexCoordY = _swapFloat;

		_swapFloat = _leftTexCoordYStep;
		_leftTexCoordYStep = _rightTexCoordYStep;
		_rightTexCoordYStep = _swapFloat;

		_swapFloat = _leftOneOverZ;
		_leftOneOverZ = _rightOneOverZ;
		_rightOneOverZ = _swapFloat;

		_swapFloat = _leftOneOverZStep;
		_leftOneOverZStep = _rightOneOverZStep;
		_rightOneOverZStep = _swapFloat;

		_swapFloat = _leftDepth;
		_leftDepth = _rightDepth;
		_rightDepth = _swapFloat;

		_swapFloat = _leftDepthStep;
		_leftDepthStep = _rightDepthStep;
		_rightDepthStep = _swapFloat;
	}

	_yStart = _middleToBottomYStart;
	_yEnd = _middleToBottomYEnd;
	_yCurrent = _yStart;
	while(_yCurrent < _yEnd)
	{
		int _xMin = (int)ceil(*_leftCurrentX);
		int _xMax = (int)ceil(*_rightCurrentX);
		float _xPrestep = _xMin - *_leftCurrentX;

		float _xDistance = *_rightCurrentX - *_leftCurrentX;
		float _texCoordXXStep = (*_rightTexCoordX - *_leftTexCoordX) / _xDistance;
		float _texCoordYXStep = (*_rightTexCoordY - *_leftTexCoordY) / _xDistance;
		float _currentOneOverZXStep = (*_rightOneOverZ - *_leftOneOverZ) / _xDistance;
		float _currentDepthStep = (_rightDepthStep - _leftDepthStep) / _xDistance;

		float _texCoordX = *_leftTexCoordX + _texCoordXXStep * _xPrestep;
		float _texCoordY = *_leftTexCoordY + _texCoordYXStep * _xPrestep;
		float _currentOneOverZ = *_leftOneOverZ + _currentOneOverZXStep * _xPrestep;
		float _currentDepth = *_leftDepth + _currentDepthStep * _xPrestep;

		int _xCurrent = _xMin;
		uint8_t* _row = (uint8_t*)(_RenderBuffer->memory) + ((uint32_t)_xCurrent * game_render_bytesPerPixel) + ((_RenderBuffer->height - (uint32_t)_yCurrent) * _RenderBuffer->pitch);    
		uint32_t* _pixel = (uint32_t*)_row;
		while(_xCurrent < _xMax)
		{
			int _depthIndex = _yCurrent * _RenderBuffer->width + _xCurrent;
			if(_currentDepth < _RenderBuffer->depthBuffer[_depthIndex]) // if what you're drawing is closer to your camera draw it
			{
				_RenderBuffer->depthBuffer[_depthIndex] = _currentDepth;
				float _z = 1.0f / _currentOneOverZ;
				int _texX = (int)((_texCoordX * _z) * (float)(_Texture.width - 1) + 0.5f);
				int _texY = (int)((_texCoordY * _z) * (float)(_Texture.height - 1) + 0.5f);
				int _colorIndex = _texY * _Texture.width + _texX;
				math_4D_vector _tempColor = _Texture.color[_colorIndex];
				uint32_t _color = (utility_round_floatToUint32(_tempColor.x * 255.0f) << 16) | (utility_round_floatToUint32(_tempColor.y * 255.0f) << 8) | (utility_round_floatToUint32(_tempColor.z * 255.0f) << 0); 
				*_pixel = _color;
			}
			_pixel++;
			_xCurrent++;
			_currentOneOverZ += _currentOneOverZXStep;
			_texCoordX += _texCoordXXStep;
			_texCoordY += _texCoordYXStep;
			_currentDepth += _currentDepthStep;
		}
		*_leftCurrentX += *_leftXStep;
		*_rightCurrentX += *_rightXStep;
		*_leftTexCoordX += *_leftTexCoordXStep;
		*_leftTexCoordY += *_leftTexCoordYStep;
		*_leftOneOverZ += *_leftOneOverZStep;
		*_rightTexCoordX += *_rightTexCoordXStep;
		*_rightTexCoordY += *_rightTexCoordYStep;
		*_rightOneOverZ += *_rightOneOverZStep;
		*_leftDepth += *_leftDepthStep;
		*_rightDepth += *_rightDepthStep;
		_yCurrent++;
	}
}



// SOUND //
// MEMORY //
void* game_memory_allocate(game_memory_allocator* _Allocator, size_t _Size)
{
	void* _result = _Allocator->base + _Allocator->used;
	_Allocator->used += _Size;
	return _result;
}


// MAIN //
extern "C" void game_main_init(uint8_t* _IsRunning, game_sound_buffer* _SoundBuffer, game_render_buffer* _RenderBuffer, game_input_buffer* _InputBuffer, game_memory_allocated* _AllocatedMemory)
{
	// INIT MEMORY //
	game_main_state* _gameState = (game_main_state*)_AllocatedMemory->permanentStorage;
	_gameState->currentCharacter = game_character_debugger;
	_gameState->currentStance = game_stance_debug;
	_gameState->currentAnimationFrame = 0;
	_gameState->numberOfFramesToCompleteCurrentMove = 0;
	_gameState->allocator.size = (size_t)_AllocatedMemory->permanentStorageSize - sizeof(game_main_state);
	_gameState->allocator.base = (uint8_t*)_AllocatedMemory->permanentStorage + sizeof(game_main_state);
	_gameState->allocator.used = sizeof(game_main_state);
	// load character data
	// TODO load this from a file
	// TODO load this into _AllocatedMemory
#if 1
	_gameState->characters[game_character_debugger].numberOfMoves = game_character_debuggerMoveCount;
	_gameState->characters[game_character_debugger].moves = (game_character_moveData*)game_memory_allocate(&_gameState->allocator, sizeof(game_character_moveData)*game_character_debuggerMoveCount);
	// first move is no move
	_gameState->characters[game_character_debugger].moves[0].buttons[0] = game_input_1;
	_gameState->characters[game_character_debugger].moves[0].buttons[1] = game_input_2;
	_gameState->characters[game_character_debugger].moves[0].buttons[2] = game_input_3;
	_gameState->characters[game_character_debugger].moves[0].buttons[3] = game_input_4;
	_gameState->characters[game_character_debugger].moves[0].numberOfFramesToComplete = 010;
	_gameState->characters[game_character_debugger].moves[0].requiredStance = game_stance_debug;
	_gameState->characters[game_character_debugger].moves[0].stance = game_stance_debug;

	_gameState->characters[game_character_debugger].moves[1].buttons[0] = game_input_2;
	_gameState->characters[game_character_debugger].moves[1].buttons[1] = game_input_3;
	_gameState->characters[game_character_debugger].moves[1].buttons[2] = game_input_4;
	_gameState->characters[game_character_debugger].moves[1].buttons[3] = game_input_1;
	_gameState->characters[game_character_debugger].moves[1].numberOfFramesToComplete = 20;
	_gameState->characters[game_character_debugger].moves[1].requiredStance = game_stance_debug;
	_gameState->characters[game_character_debugger].moves[1].stance = game_stance_debug;
#endif


	// INIT INPUT //
	// clear 
	_InputBuffer->queuedMoveId = game_input_none;
	_InputBuffer->moveBufferIndex = 0;
	_InputBuffer->framesSinceLastInput = 0;
	game_input_clearButtons(_InputBuffer->buttonsPressedDuringFrame);
	game_input_clearMoveBuffer(_InputBuffer->moveBuffer);
	int _keybindIndex = 0;
	while(_keybindIndex < game_input_numberOfMovementBinds)
	{
		_InputBuffer->movementKeyEndedDown[_keybindIndex] = game_input_endedUp;
		_InputBuffer->movementKeyTransitionCount[_keybindIndex] = 0;
		_keybindIndex++;
	}


	// INIT MENU //
	_gameState->isInMenu = true;


	// INIT RENDER //
	_RenderBuffer->depthBuffer  = (float*)game_memory_allocate(&_gameState->allocator, sizeof(float) * _RenderBuffer->width * _RenderBuffer->height);
	float _aspectRatio = (float)_RenderBuffer->width / (float)_RenderBuffer->height;
	float _fieldOfView = math_constant_degreesPerRadian * 90.0f;
	float _zNear = 0.1f;
	float _zFar = 1000.0f;
	_gameState->cameraProjectionMatrix = math_4D_createPerspectiveMatrix(_fieldOfView, _aspectRatio, _zNear, _zFar);
	_gameState->objectPositionData.position = {0.0f,0.0f,3.0f,0.0f};
	_gameState->objectPositionData.rotation = {0.0f,0.0f,0.0f,1.0f};
	_gameState->objectPositionData.scale = {1.0f,1.0f,1.0f,1.0f};
	_gameState->cameraPositionData.position = {0.0f,0.0f,0.0f,0.0f};
	_gameState->cameraPositionData.rotation = {0.0f,0.0f,0.0f,1.0f};
	_gameState->cameraPositionData.scale = {1.0f,1.0f,1.0f,1.0f};
	_gameState->rotationCounter = 0;

	// TODO load texture from file
	// current texture x as index of color
	// current texture y as index of color
	//_gameState->objectTexture.colorXStep = {0.0f, 0.1f, 0.0f, 0.0f}; // calculate this based on 
	//_gameState->objectTexture.colorYStep = {0.0f, 0.0f, 0.1f, 0.0f};
	_gameState->objectTexture.width = 2;
	_gameState->objectTexture.height = 2;
	_gameState->objectTexture.color = (math_4D_vector*)game_memory_allocate(&_gameState->allocator, sizeof(game_character_moveData) * _gameState->objectTexture.width * _gameState->objectTexture.height);
	_gameState->objectTexture.color[0] = {0.2f, 0.2f, 0.2f, 0.0f}; // top left
	_gameState->objectTexture.color[1] = {0.2f, 0.2f, 0.2f, 0.0f}; // top right
	_gameState->objectTexture.color[2] = {0.2f, 0.2f, 0.2f, 0.0f}; // middle left
	_gameState->objectTexture.color[3] = {0.0f, 0.0f, 0.0f, 0.0f}; // middle right
	//_gameState->objectTexture.color[4] = {1.0f, 1.0f, 1.0f, 0.0f}; // bottom left
	//_gameState->objectTexture.color[5] = {0.0f, 0.0f, 0.0f, 0.0f}; // bottom right


	// INIT SOUND //
	_SoundBuffer->toneHz = 256;
	_SoundBuffer->tSine = 0.0f;
}

extern "C" void game_main_update(uint8_t* _IsRunning, game_sound_buffer* _SoundBuffer, game_render_buffer* _RenderBuffer, game_input_buffer* _InputBuffer, game_memory_allocated* _AllocatedMemory)
{
	// UPDATE MEMORY //
	game_main_state* _gameState = (game_main_state*)_AllocatedMemory->permanentStorage;
	if(_gameState->platformMessage && game_memory_loadedFile)
	{
		// TODO: make the objectVerticies 4D and calculate normals 
		_gameState->objectVerticies = (math_4D_vector*)game_memory_allocate(&_gameState->allocator, sizeof(math_4D_vector));
		_gameState->objectVerticiesCount = 0;
		uint8_t* _fileContents = (uint8_t*)_AllocatedMemory->transientStorage;
		uint64_t _fileSize = _AllocatedMemory->transientStorageSize;
		uint64_t _fileCounter = 0;
		int _verticiesCounted = false;
		while(*_fileContents != NULL && _fileCounter < _fileSize)
		{
			_fileCounter++;
			if(*_fileContents == 'v' && *(_fileContents+1) == ' ') // v not vn
			{
				game_memory_allocate(&_gameState->allocator, sizeof(math_4D_vector));
				// process line
				int _isNegative = false;
				int _decimalPosition = 0;
				float _currentNumber = 0;
				int _numbersProcessed = 0;
				math_4D_vector* _currentVector = _gameState->objectVerticies + _gameState->objectVerticiesCount;
				_gameState->objectVerticiesCount++;
				_fileContents++;
				_fileContents++;
				_fileContents++;
				while(_fileContents != NULL && *_fileContents != '\n')
				{
					if(*_fileContents == ' ')
					{
						int _divisor = 1;
						while(_decimalPosition > 0)
						{
							_divisor = _divisor * 10;
							_decimalPosition--;
						}
						_currentNumber = _currentNumber / _divisor;
						if(_isNegative)
						{
							_currentNumber = -_currentNumber;
						}
						if(_numbersProcessed == 0)
						{
							_currentVector->x = _currentNumber;
						}
						else
						{
							_currentVector->y = _currentNumber;
						}
						_isNegative = false;
						_decimalPosition = 0;
						_numbersProcessed++;
						_fileContents++;
						_fileContents++;
						_decimalPosition = -2;
						_currentNumber = 0;
						continue;
					}
					else if(*_fileContents == '-')
					{
						_isNegative = true;
					}
					else if(*_fileContents == '.')
					{
						_decimalPosition = -1;
					}
					else 
					{
						_currentNumber = _currentNumber * 10 + (*_fileContents - '0');
					}
					if(_decimalPosition >= -1)
					{
						_decimalPosition++;
					}
					_fileContents++;
				}
				int _divisor = 1;
				while(_decimalPosition > 0)
				{
					_divisor = _divisor * 10;
					_decimalPosition--;
				}
				_currentNumber = _currentNumber / _divisor;
				if(_isNegative)
				{
					_currentNumber = -_currentNumber;
				}
				_currentVector->z = _currentNumber;
				_currentVector->w = 1.0f;
			}
			else if(*_fileContents == 'f' && *(_fileContents+1) == ' ') 
			{
				if(!_verticiesCounted)
				{
					_gameState->triangleFaces = (game_render_face*) game_memory_allocate(&_gameState->allocator, sizeof(game_render_face));
					_verticiesCounted = true;
				}
				game_memory_allocate(&_gameState->allocator, sizeof(game_render_face));
				// process line
				// TODO get rid of _gameState temp variables
				int _isNegative = false;
				int _decimalPosition = 0;
				int _currentNumber = 0;
				int _numbersProcessed = 0;
				game_render_face* _currentTriangle = _gameState->triangleFaces + _gameState->triangleCount;
				_gameState->triangleCount++;
				_fileContents++;
				_fileContents++;
				_fileContents++;
				while(_fileContents != NULL && *_fileContents != '\n' && *_fileContents != '\r')
				{
					if(*_fileContents == ' ')
					{
						if(_numbersProcessed == 0)
						{
							_currentTriangle->vertex1 = _currentNumber;
						}
						else
						{
							_currentTriangle->vertex2 = _currentNumber;
						}
						_numbersProcessed++;
						_fileContents++;
						_fileContents++;
						_currentNumber = 0;
						continue;
					}
					else 
					{
						_currentNumber = _currentNumber * 10 + (*_fileContents - '0');
					}
					_fileContents++;
				}
				_currentTriangle->vertex3 = _currentNumber;
			}
			_fileContents++;
		}
		_gameState->platformMessage = _gameState->platformMessage & !game_memory_loadedFile;
	}
	float _playerSpeed = _gameState->secondsPassed*500.0f;


	// UPDATE SOUND //
	int16_t _toneVolume = 0000;
#if 0
	int _wavePeriod = _SoundBuffer->samplesPerSecond / _SoundBuffer->toneHz;
	int16_t* _sampleOut = _SoundBuffer->samples;
	int _sampleIndex = 0;
	while(_sampleIndex < _SoundBuffer->sampleCount)
	{
		float _sineValue = sinf(_SoundBuffer->tSine);
		int16_t _sampleValue = (int16_t)(_sineValue * _toneVolume);
		*_sampleOut = _sampleValue;
		_sampleOut++;
		*_sampleOut = _sampleValue;
		_sampleOut++;

		_SoundBuffer->tSine += (2.0f * math_constant_pifloat) / (float)_wavePeriod;
		if(_SoundBuffer->tSine > (2.0f * math_constant_pifloat))
		{
			_SoundBuffer->tSine -= 2.0f * math_constant_pifloat;
		}
		_sampleIndex++;
	}
#endif

	// UPDATE INPUT //
	if(_gameState->isInMenu)
	{
		uint8_t _buttonIndex = 0;
		// proccess actions
		while(_InputBuffer->buttonsPressedDuringFrame[_buttonIndex] != game_input_none && _buttonIndex < game_input_numberOfButtonsPerFrame)
		{
			if(_InputBuffer->buttonsPressedDuringFrame[_buttonIndex] == game_input_1)
			{
				_gameState->playerReset = 1;
				if(_gameState->menuIndex == 0)
				{
					_gameState->isInMenu = false;
				}
				else if(_gameState->menuIndex == 1)
				{
					*_IsRunning = false;
				}
			}
			if(_InputBuffer->buttonsPressedDuringFrame[_buttonIndex] == game_input_2)
			{
				// LOAD OBJECT
				_gameState->platformMessage = game_memory_loadFile;
				_gameState->loadFileType = game_file_object;
				_gameState->loadFileIndex = 0;
			}
			if(_InputBuffer->buttonsPressedDuringFrame[_buttonIndex] == game_input_3)
			{
			}
			if(_InputBuffer->buttonsPressedDuringFrame[_buttonIndex] == game_input_4)
			{
			}
			_buttonIndex++;
		}
		if(_InputBuffer->movementKeyTransitionCount[game_input_up] > 0)
		{
			_gameState->menuIndex++;
			if(_gameState->menuIndex >= game_menu_numberOfMenuOptions)
			{
				_gameState->menuIndex = 0;
			}
			_InputBuffer->movementKeyTransitionCount[game_input_up]--;
		}
		if(_InputBuffer->movementKeyTransitionCount[game_input_down] > 0)
		{
			if(_gameState->menuIndex == 0)
			{
				_gameState->menuIndex = game_menu_numberOfMenuOptions;
			}
			_gameState->menuIndex--;
			_InputBuffer->movementKeyTransitionCount[game_input_down]--;
		}
		if(_InputBuffer->movementKeyEndedDown[game_input_left] == game_input_endedDown)
		{
		}
		if(_InputBuffer->movementKeyEndedDown[game_input_right] == game_input_endedDown)
		{
		}
	}
	else
	{ // In game
		// Process Buttons Pressed into a move
		int8_t _inputBufferHasAButtonToProcess = false;
		int8_t _buttonIndex = 0;
		if(_InputBuffer->buttonsPressedDuringFrame[0] != game_input_none)
		{
			_inputBufferHasAButtonToProcess = true;
		}
		if(_inputBufferHasAButtonToProcess)
		{
			if(_InputBuffer->buttonsPressedDuringFrame[0] == game_input_escape)
			{
				_gameState->isInMenu = true;
				_gameState->currentMoveListIndex = game_input_none;
				_gameState->numberOfFramesToCompleteCurrentMove = 0;
				_gameState->currentStance = game_stance_debug;
				_InputBuffer->queuedMoveId = game_input_none;
			}
			if(_InputBuffer->framesSinceLastInput > game_input_frameThreshholdBetweenInputs)
			{ // failed move took too long
				game_input_clearMoveBuffer(_InputBuffer->moveBuffer);
				_InputBuffer->framesSinceLastInput = 0;
				_InputBuffer->moveBufferIndex = 0;
			}

			uint8_t _noButtonAddedToMove = true;
			_buttonIndex = 0;
			while(_buttonIndex < game_input_numberOfButtonsPerFrame)
			{ // process all buttons pressed last frame

				if(_InputBuffer->buttonsPressedDuringFrame[_buttonIndex] == game_input_none)
				{ // no buttons left to process
					break;
				}

				// check if button is unique
				uint8_t _moveBufferIndex = 0;
				uint8_t _buttonIsUnique = true;
				while(_moveBufferIndex < game_input_numberOfButtonsPerMove)
				{
					if(_InputBuffer->moveBuffer[_moveBufferIndex] == _InputBuffer->buttonsPressedDuringFrame[_buttonIndex])
					{
						_buttonIsUnique = false;
						break;
					}
					_moveBufferIndex++;
				}

				if(_buttonIsUnique)
				{ // add button to moveBuffer
					_InputBuffer->moveBuffer[_InputBuffer->moveBufferIndex] = _InputBuffer->buttonsPressedDuringFrame[_buttonIndex];
					_InputBuffer->moveBufferIndex++;
					if(_InputBuffer->moveBufferIndex >= game_input_numberOfButtonsPerMove)
					{ // add move to queue to process it 
						if(_InputBuffer->queuedMoveId == game_input_none) 
						{ // Process move in moveBuffer to check if it's a valid move to queue
							game_character_data _characterData = _gameState->characters[_gameState->currentCharacter];
							uint8_t _moveListIndex = 0;
							uint8_t _numberOfMoves = _characterData.numberOfMoves;
							uint8_t _isAMoveOnMoveList;
							while(_moveListIndex < _numberOfMoves)
							{
								_isAMoveOnMoveList = true;
								_buttonIndex = 0;
								while(_buttonIndex < game_input_numberOfButtonsPerMove)
								{
									if(_characterData.moves[_moveListIndex].buttons[_buttonIndex] != _InputBuffer->moveBuffer[_buttonIndex])
									{
										_isAMoveOnMoveList = false;
									}
									_buttonIndex++;
								}

								if(_isAMoveOnMoveList)
								{
									// check if move is valid given game conditions
									if(_gameState->currentStance == _characterData.moves[_moveListIndex].requiredStance)
									{
										_InputBuffer->queuedMoveId = _moveListIndex;
									}
									else
									{
										_InputBuffer->queuedMoveId = game_input_none;
									}

									break;
								}
								_moveListIndex++;
							}
						}
						// clear buffer for new inputs
						_InputBuffer->moveBufferIndex = 0;
						game_input_clearMoveBuffer(_InputBuffer->moveBuffer);
					}
				}
				_buttonIndex++;
			}
			if(_noButtonAddedToMove)
			{ // no unique buttons pressed last frame
				_InputBuffer->framesSinceLastInput++;	
			}
		}
		else
		{ // no buttons pressed last frame
			_InputBuffer->framesSinceLastInput++;	
		}

		// process moves and increment frame counter
		if(_gameState->currentAnimationFrame < _gameState->numberOfFramesToCompleteCurrentMove)
		{
			if(_gameState->currentMoveListIndex == 0)
			{
				_gameState->playerMomentum = 0;
				_gameState->playerPosition.x = (float)_RenderBuffer->width/2;
				_gameState->playerPosition.y = (float)_RenderBuffer->height/2;
			}
			else if(_gameState->currentMoveListIndex == 1)
			{
				_gameState->playerMomentum += 5;
			}
			_gameState->currentAnimationFrame++;
		}
		else if(_InputBuffer->queuedMoveId != game_input_none)
		{ // process next move in queue
			_gameState->currentAnimationFrame = 0;
			_gameState->currentMoveListIndex = _InputBuffer->queuedMoveId;
			_gameState->numberOfFramesToCompleteCurrentMove = _gameState->characters[_gameState->currentCharacter].moves[_gameState->currentMoveListIndex].numberOfFramesToComplete;
			_gameState->currentStance = _gameState->characters[_gameState->currentCharacter].moves[_gameState->currentMoveListIndex].stance;
			_InputBuffer->queuedMoveId = game_input_none;
		}
		else // move is over clear gameState
		{
			_gameState->currentAnimationFrame = 0;
			_gameState->currentMoveListIndex = game_input_none;
			_gameState->numberOfFramesToCompleteCurrentMove = 0;
			_gameState->currentStance = game_stance_debug;
		}

		// Process movement Keys / mouse
#if 0
		_gameState->playerX = (float)(_InputBuffer->mouseX) ;
		_gameState->playerY = (float)(_InputBuffer->mouseY) ;
#endif
		float _speed = _gameState->playerMomentum * _gameState->secondsPassed;
		int _movementKeyIndex = 0;
		while(_movementKeyIndex < game_input_numberOfMovementBinds)
		{
			if(_InputBuffer->movementKeyEndedDown[_movementKeyIndex] != 0)
			{
				switch(_movementKeyIndex)
				{
					case game_input_up:
						{
							_gameState->playerDirection.x = 0;
							_gameState->playerDirection.y = 1;
						}
						break;
					case game_input_down:
						{
							_gameState->playerDirection.x = 0;
							_gameState->playerDirection.y = -1;
						}
						break;
					case game_input_left:
						{
							_gameState->playerDirection.x = -1;
							_gameState->playerDirection.y = 0;
						}
						break;
					case game_input_right:
						{
							_gameState->playerDirection.x = 1;
							_gameState->playerDirection.y = 0;
						}
						break;
				};
				_gameState->playerDirection =  math_2D_multiplyVectorByScalar(_gameState->playerDirection , _speed);
				_gameState->playerPosition =  math_2D_addVectors(_gameState->playerDirection, _gameState->playerPosition);
			}
			_movementKeyIndex++;
		}
	}
	game_input_clearButtons(_InputBuffer->buttonsPressedDuringFrame);


	// UPDATE RENDER //
	if(_gameState->isInMenu)
	{
		math_4D_vector _backgroundColor = {0.0f, 0.0f, 0.0f, 0.0f}; 
		game_render_rectangle(_RenderBuffer, 0, 0, (float)_RenderBuffer->width, (float)_RenderBuffer->height, _backgroundColor);

		float _menuItemWidth = (float)_RenderBuffer->width / 3;
		float _leftPadding = _menuItemWidth;
		float _paddingBetweenMenuItems = (float)_RenderBuffer->height / 18;
		float _menuItemHeight = (float)_RenderBuffer->height / (game_menu_numberOfMenuOptions + 1) - _paddingBetweenMenuItems;
		float _selectedPadding = 15.0f;

		float _top = 0;
		float _previousTop = _top;
		math_4D_vector _menuItemColor = {1.0f, 1.0f, 0.0f};
		math_4D_vector _selectedColor = {1.0f, 1.0f, 1.0f};

		int _menuItemIndex = game_menu_numberOfMenuOptions-1;
		while(_menuItemIndex >= 0)
		{
			_menuItemColor.x =  1 /(float)(_menuItemIndex+1);
			float _left = _leftPadding;
			float _right = _left + _menuItemWidth;
			float _bottom = _top + _menuItemHeight;
			if(_menuItemIndex == _gameState->menuIndex)
			{
				game_render_rectangle(_RenderBuffer, _left-_selectedPadding, _top-_selectedPadding, _right+_selectedPadding, _bottom+_selectedPadding, _selectedColor);
			}
			game_render_rectangle(_RenderBuffer, _left, _top, _right, _bottom, _menuItemColor);
			_top = _previousTop + _menuItemHeight + _paddingBetweenMenuItems;
			_previousTop = _top;
			_menuItemIndex--;
		}
	}
	else
	{
		math_4D_vector _backgroundColor = {};
		game_render_rectangle(_RenderBuffer, 0, 0, (float)_RenderBuffer->width, (float)_RenderBuffer->height, _backgroundColor);

		/*
		math_4D_vector _vector1 = {0,0,(float)_RenderBuffer->width/1,1};
		math_4D_vector _vector2 = {0,(float)_RenderBuffer->height/2,0,1};
		math_4D_vector _vector3 = {(float)_RenderBuffer->width/2,0,0,1};
		*/

		math_4D_vector _triangleFillColor = {1, 0, 1};
		//game_render_triangle(_RenderBuffer, _vector1, _vector2, _vector3, _triangleFillColor);

		float _screenWidth = (float)_RenderBuffer->width; 
		float _screenHeight = (float)_RenderBuffer->height;
		float _aspectRatio = _screenWidth / _screenHeight;
		_gameState->secondsPassed;
		// UPDATE CAMERA //
		// TODO: get these from settings
		_gameState->cameraPositionData.position = {0.0f,0.0f,0.0f,0.0f};
		_gameState->cameraPositionData.rotation = {0.0f,0.0f,0.0f,1.0f};
		_gameState->cameraPositionData.scale = {1.0f,1.0f,1.0f,1.0f};

		int _moveCamera = false;
		game_camera_moveDirection _cameraMoveDirection = game_camera_moveLeft; // TODO get this from INPUT
		if(_moveCamera)
		{
			float _moveAmount = 5.0f * _gameState->secondsPassed;
			// select correct direction
			math_4D_vector _selectedDirectionVector = math_4D_leftVector;
			if(_cameraMoveDirection == game_camera_moveLeft)
			{
				_selectedDirectionVector = math_4D_leftVector;
			}
			if(_cameraMoveDirection == game_camera_moveRight)
			{
				_selectedDirectionVector = math_4D_rightVector;
			}
			if(_cameraMoveDirection == game_camera_moveForward)
			{
				_selectedDirectionVector = math_4D_forwardVector;
			}
			if(_cameraMoveDirection == game_camera_moveBackward)
			{
				_selectedDirectionVector = math_4D_backwardVector;
			}

			math_4D_vector _rotatedDirectionVector = math_4D_rotate(_gameState->cameraPositionData.rotation, _selectedDirectionVector);
			_rotatedDirectionVector = math_4D_scale(_rotatedDirectionVector, _moveAmount);
			_gameState->cameraPositionData.position =  math_4D_add(_gameState->cameraPositionData.position, _rotatedDirectionVector);
		}

		// TODO: get this from INPUT
		int _rotateCamera = false;
		game_camera_rotateDirection _rotateDirection = game_camera_rotateLeft;
		if(_rotateCamera)
		{
			float _sensitivityX = 2.66f * _gameState->secondsPassed;
			float _sensitivityY = 2.0f * _gameState->secondsPassed;
			math_4D_vector _yAxis = {0,1,0,1};
			math_quaternion_struct _rotation = {};  
			if(_rotateDirection == game_camera_rotateLeft)
			{
				float _sinHalfAngle = (float)sin(_sensitivityX/2);
				float _cosHalfAngle = (float)cos(_sensitivityX/2);
				_rotation.x = _yAxis.x * _sinHalfAngle;
				_rotation.y = _yAxis.y * _sinHalfAngle;
				_rotation.z = _yAxis.z * _sinHalfAngle;
				_rotation.w = _cosHalfAngle;
			}
			else if(_rotateDirection == game_camera_rotateRight)
			{
				// rotate right
				float _sinHalfAngle = (float)sin(-_sensitivityX/2);
				float _cosHalfAngle = (float)cos(-_sensitivityX/2);
				_rotation.x = _yAxis.x * _sinHalfAngle;
				_rotation.y = _yAxis.y * _sinHalfAngle;
				_rotation.z = _yAxis.z * _sinHalfAngle;
				_rotation.w = _cosHalfAngle;
			}
			else if(_rotateDirection == game_camera_rotateUp)
			{
				// rotate up
				float _sinHalfAngle = (float)sin(_sensitivityY/2);
				float _cosHalfAngle = (float)cos(_sensitivityY/2);
				math_4D_vector _rotatedDirectionVector = math_4D_rotate(_gameState->cameraPositionData.rotation, math_4D_rightVector);
				_rotation.x = _rotatedDirectionVector.x * _sinHalfAngle;
				_rotation.y = _rotatedDirectionVector.y * _sinHalfAngle;
				_rotation.z = _rotatedDirectionVector.z * _sinHalfAngle;
				_rotation.w = _cosHalfAngle;
			}
			else if(_rotateDirection == game_camera_rotateDown)
			{
				// rotate down
				float _sinHalfAngle = (float)sin(-_sensitivityY/2);
				float _cosHalfAngle = (float)cos(-_sensitivityY/2);
				math_4D_vector _rotatedDirectionVector = math_4D_rotate(_gameState->cameraPositionData.rotation, math_4D_rightVector);
				_rotation.x = _rotatedDirectionVector.x * _sinHalfAngle;
				_rotation.y = _rotatedDirectionVector.y * _sinHalfAngle;
				_rotation.z = _rotatedDirectionVector.z * _sinHalfAngle;
				_rotation.w = _cosHalfAngle;
			}
			math_quaternion_struct _newcameraRotation = math_quaternion_multiplyTwoQuaternions(_rotation, _gameState->cameraPositionData.rotation);
			_newcameraRotation = math_quaternion_normalize(_newcameraRotation);
		}


		// UPDATE OBJECTS //
#if 1
#if 0
		_gameState->objectVerticies = (math_4D_vector*)game_memory_allocate(&_gameState->allocator, 3*sizeof(math_4D_vector));
		(_gameState->objectVerticies)->x = -1;
		(_gameState->objectVerticies)->y = -1;
		(_gameState->objectVerticies)->z = 0;
		(_gameState->objectVerticies)->w = 1;

		(_gameState->objectVerticies+1)->x = 0;
		(_gameState->objectVerticies+1)->y = 1;
		(_gameState->objectVerticies+1)->z = 0;
		(_gameState->objectVerticies+1)->w = 1;

		(_gameState->objectVerticies+2)->x = 1;
		(_gameState->objectVerticies+2)->y = -1;
		(_gameState->objectVerticies+2)->z = 0;
		(_gameState->objectVerticies+2)->w = 1;

		_gameState->objectVerticiesCount = 3;
#endif
#if 0
		_aspectRatio = (float)_RenderBuffer->width / (float)_RenderBuffer->height;
		float _fieldOfView = math_constant_degreesPerRadian * 70.0f;
		float _zNear = 0.1f;
		float _zFar = 1000.0f;
		_gameState->cameraProjectionMatrix = math_4D_createPerspectiveMatrix(_fieldOfView, _aspectRatio, _zNear, _zFar);
#endif
		// normalize verticies

		// RENDER OBJECTS //
		// Clear depth buffer
		{
			int _depthBufferIndex = 0;
			int _depthBufferSize = _RenderBuffer->width * _RenderBuffer->height;
			while(_depthBufferIndex < _depthBufferSize)
			{
				_RenderBuffer->depthBuffer[_depthBufferIndex] = FLT_MAX;
				_depthBufferIndex++;
			}
		}
		math_4D_vector* _verticies = _gameState->objectVerticies;
		float _largestLength = 0;
		uint32_t _vertexCounter = 0;
		while(_vertexCounter < _gameState->objectVerticiesCount)
		{
			math_4D_vector _currentVertex = *_verticies;
			float _currentLength = (float)sqrt(_currentVertex.x * _currentVertex.x + _currentVertex.y * _currentVertex.y + _currentVertex.z * _currentVertex.z);
			if(_currentLength > _largestLength)
			{
				_largestLength = _currentLength;
			}
			_verticies++;
			_vertexCounter++;
		}
		_verticies = _gameState->objectVerticies;
		_vertexCounter = 0;
		while(_vertexCounter < _gameState->objectVerticiesCount)
		{
			_verticies->x = _verticies->x / _largestLength;
			_verticies->y = _verticies->y / _largestLength;
			_verticies->z = _verticies->z / _largestLength;
			_verticies++;
			_vertexCounter++;
		}

		_gameState->rotationCounter += _gameState->secondsPassed;
		math_4D_matrix _screenSpaceMatrix = math_4D_createScreenSpaceMatrix(_screenWidth / 2, _screenHeight / 2);
		math_4D_matrix _translationMatrix = math_4D_createTranslationMatrix(0.0f, 0.0f, 2.0f);
		math_4D_matrix _rotationMatrix = math_4D_createRotationMatrix(_gameState->rotationCounter ,_gameState->rotationCounter, _gameState->rotationCounter );//_gameState->rotationCounter
		math_4D_matrix _rotatedMatrix = math_4D_multiplyTwoMatricies(_translationMatrix, _rotationMatrix);
		_rotatedMatrix = math_4D_multiplyTwoMatricies(_gameState->cameraProjectionMatrix, _rotatedMatrix);
		uint32_t _faceCounter = 0;
		game_render_face* _currentTriangle = _gameState->triangleFaces;
#if 0
		game_render_vertex _vertex1;
		game_render_vertex _vertex2;
		game_render_vertex _vertex3;
		_vertex1.position = {-1.0f, -1.0f, 0.0f, 1.0f};
		_vertex2.position = {0.0f, 1.0f, 0.0f, 1.0f};
		_vertex3.position = {1.0f, -1.0f, 0.0f, 1.0f};
		_vertex1.position = math_4D_transformVectorByMatrix(_vertex1.position, _rotatedMatrix);
		_vertex2.position = math_4D_transformVectorByMatrix(_vertex2.position, _rotatedMatrix);
		_vertex3.position = math_4D_transformVectorByMatrix(_vertex3.position, _rotatedMatrix);
		_vertex1.position = math_4D_transformVectorByMatrix(_vertex1.position, _screenSpaceMatrix);
		_vertex2.position = math_4D_transformVectorByMatrix(_vertex2.position, _screenSpaceMatrix);
		_vertex3.position = math_4D_transformVectorByMatrix(_vertex3.position, _screenSpaceMatrix);
		_vertex1.position = math_4D_divideByPerspective(_vertex1.position);
		_vertex2.position = math_4D_divideByPerspective(_vertex2.position);
		_vertex3.position = math_4D_divideByPerspective(_vertex3.position);
		_vertex1.textureCoordinates = {0.0f, 0.0f};
		_vertex2.textureCoordinates = {0.5f, 1.0f};
		_vertex3.textureCoordinates = {1.0f, 0.0f};
		game_render_triangle(_RenderBuffer, _vertex1, _vertex2, _vertex3, _gameState->objectTexture);
#else
		while(_faceCounter < _gameState->triangleCount)
		{
			game_render_vertex _vertex1;
			game_render_vertex _vertex2;
			game_render_vertex _vertex3;
			_vertex1.position = *(_gameState->objectVerticies + _currentTriangle->vertex1-1);
			_vertex2.position = *(_gameState->objectVerticies + _currentTriangle->vertex2-1);
			_vertex3.position = *(_gameState->objectVerticies + _currentTriangle->vertex3-1);
			_vertex1.position = math_4D_transformVectorByMatrix(_vertex1.position, _rotatedMatrix);
			_vertex2.position = math_4D_transformVectorByMatrix(_vertex2.position, _rotatedMatrix);
			_vertex3.position = math_4D_transformVectorByMatrix(_vertex3.position, _rotatedMatrix);
			_vertex1.position = math_4D_transformVectorByMatrix(_vertex1.position, _screenSpaceMatrix);
			_vertex2.position = math_4D_transformVectorByMatrix(_vertex2.position, _screenSpaceMatrix);
			_vertex3.position = math_4D_transformVectorByMatrix(_vertex3.position, _screenSpaceMatrix);
			_vertex1.position = math_4D_divideByPerspective(_vertex1.position);
			_vertex2.position = math_4D_divideByPerspective(_vertex2.position);
			_vertex3.position = math_4D_divideByPerspective(_vertex3.position);
			_vertex1.textureCoordinates = {0.0f, 0.0f};
			_vertex2.textureCoordinates = {0.5f, 1.0f};
			_vertex3.textureCoordinates = {1.0f, 0.0f};
			game_render_triangle(_RenderBuffer, _vertex1, _vertex2, _vertex3, _gameState->objectTexture);
			_currentTriangle++;
			_faceCounter++;
		}
#endif
#else 
		// construct upward rotation
		math_quaternion_struct _rotation;
		float sinHalfAngle = (float)sin(_gameState->secondsPassed / 2);
		float cosHalfAngle = (float)cos(_gameState->secondsPassed / 2);
		_rotation.x = 0 * sinHalfAngle;
		_rotation.y = 1 * sinHalfAngle;
		_rotation.z = 0 * sinHalfAngle;
		_rotation.w = cosHalfAngle;

		math_quaternion_struct _object = _gameState->objectPositionData.rotation;
		_object = math_quaternion_multiplyTwoQuaternions(_rotation, _object);
		_gameState->objectPositionData.rotation = math_quaternion_normalize(_object);

		// UPDATE CAMERA //
		math_quaternion_struct _cameraRotation = _gameState->cameraPositionData.rotation;
		_cameraRotation.x = -_cameraRotation.x;
		_cameraRotation.y = -_cameraRotation.y;
		_cameraRotation.z = -_cameraRotation.z;
		_cameraRotation.w = -_cameraRotation.w;

		math_4D_matrix _cameraRotationMatrix = math_4D_createRotationMatrix(_cameraRotation);
		math_4D_vector _cameraPosition = math_4D_scale(_gameState->cameraPositionData.position, -1);
		math_4D_matrix _cameraTranslationMatrix = math_4D_createTranslationMatrix(_cameraPosition.x, _cameraPosition.y, _cameraPosition.z);
		math_4D_matrix _cameraProjectionMatrix = _gameState->cameraProjectionMatrix;

		math_4D_matrix _resultingMatrix = math_4D_multiplyTwoMatricies(_cameraRotationMatrix, _cameraTranslationMatrix);
		_gameState->cameraProjectionMatrix = math_4D_multiplyTwoMatricies(_cameraProjectionMatrix, _resultingMatrix);

		_gameState->objectPositionData.position = {0.0f,0.0f,3.0f,0.0f};
		_gameState->objectPositionData.rotation = {0.0f,0.0f,0.0f,1.0f};
		_gameState->objectPositionData.scale = {1.0f,1.0f,1.0f,1.0f};

		// loop this VVV for each object
		math_4D_matrix _objectTranslationMatrix = math_4D_createTranslationMatrix(_gameState->objectPositionData.position.x, _gameState->objectPositionData.position.y, _gameState->objectPositionData.position.z);
		math_4D_matrix _objectRotationMatrix = math_4D_createRotationMatrix(_gameState->objectPositionData.rotation);
		math_4D_matrix _objectScaleMatrix = math_4D_createScaleMatrix(_gameState->objectPositionData.scale.x, _gameState->objectPositionData.scale.y, _gameState->objectPositionData.scale.z);
		math_4D_matrix _rotationScaled = math_4D_multiplyTwoMatricies(_objectRotationMatrix, _objectScaleMatrix);
		math_4D_matrix _objectTransformationMatrix = math_4D_multiplyTwoMatricies(_objectTranslationMatrix, _rotationScaled);
		//math_4D_matrix _drawTransformationMatrix = math_4D_multiplyTwoMatricies(_gameState->cameraProjectionMatrix, _objectTransformationMatrix);
		math_4D_matrix _drawTransformationMatrix = _objectTransformationMatrix;

		uint32_t _objectVertexIndex = 0;
		math_4D_vector* _objectVerticies = _gameState->objectVerticies;
		while(_objectVertexIndex < _gameState->objectVerticiesCount)
		{
			// draw
			math_4D_matrix _screenSpaceMatrix = math_4D_createScreenSpaceMatrix(_screenWidth / 2, _screenHeight / 2);
			math_4D_matrix _identityMatrix = math_4D_createIdentityMatrix();
			math_4D_vector _vertex1 = *(_objectVerticies);
			math_4D_vector _vertex2 = *(_objectVerticies+1);
			math_4D_vector _vertex3 = *(_objectVerticies+2);
			if (math_4D_vectorIsInsideViewFrustum(_vertex1) && math_4D_vectorIsInsideViewFrustum(_vertex2) && math_4D_vectorIsInsideViewFrustum(_vertex3))
			{ // FillTriangle(v1, v2, v3, texture);
				_vertex1 = math_4D_transformVectorByMatrix(_vertex1, _screenSpaceMatrix);
				_vertex2 = math_4D_transformVectorByMatrix(_vertex2, _screenSpaceMatrix);
				_vertex3 = math_4D_transformVectorByMatrix(_vertex3, _screenSpaceMatrix);
				_vertex1 = math_4D_divideByPerspective(_vertex1);
				_vertex2 = math_4D_divideByPerspective(_vertex2);
				_vertex3 = math_4D_divideByPerspective(_vertex3);
				math_4D_vector _triangleColor = {1.0f, 1.0f, 0.0f};
				game_render_triangle(_RenderBuffer, _vertex1, _vertex2, _vertex3, _triangleColor);
			}
#if 0
			else
			{
				// TODO this else statement tomorrow
				// ????
				List<Vertex> vertices = new ArrayList<>();
				List<Vertex> auxillaryList = new ArrayList<>();

				vertices.add(v1);
				vertices.add(v2);
				vertices.add(v3);

				if(ClipPolygonAxis(vertices, auxillaryList, 0) &&
						ClipPolygonAxis(vertices, auxillaryList, 1) &&
						ClipPolygonAxis(vertices, auxillaryList, 2))
				{
					Vertex initialVertex = vertices.get(0);

					for(int i = 1; i < vertices.size() - 1; i++)
					{
						FillTriangle(initialVertex, vertices.get(i), vertices.get(i + 1), texture);
					}
				}
			}
#endif
			_objectVerticies += 3;
			_objectVertexIndex += 3;
		}
#endif
	}
	// end of objects draw loop
}
