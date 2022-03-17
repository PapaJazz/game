#ifndef utility_h
#define utility_h

// CHARACTER //
char utility_char_intToChar(int _integer)
{
	return ('0'+(_integer%10));
}
void utility_char_replaceInto(char* _From, char* _Into)
{
	int _index = 0;
	while(_From[_index])
	{
		_Into[_index] = _From[_index];
		_index++;
	}
}
void utility_char_replaceIntoAt(char* _From, char* _Into, int _ToIndex)
{
	int _fromIndex = 0;
	while(_From[_fromIndex])
	{
		_Into[_ToIndex] = _From[_fromIndex];
		_ToIndex++; 
		_fromIndex++;
	}
}
void utility_char_replaceIntoWithTwoDigitsAt(int _Digits, char* _Into, int _Index)
{
	_Into[_Index] = utility_char_intToChar(_Digits/10);
	_Index++;
	_Into[_Index] = utility_char_intToChar(_Digits);
}

// ROUNDING //
int32_t utility_round_floatToInt32(float _Float)
{
   int32_t _returnValue = (int32_t)(_Float + 0.5f);
	return _returnValue;
}

uint32_t utility_round_floatToUint32(float _Float)
{
   uint32_t _returnValue = (uint32_t)(_Float + 0.5f);
	return _returnValue;
}

#endif
