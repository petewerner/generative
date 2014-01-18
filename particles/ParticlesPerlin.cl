float Cosine_Interpolate(float a, float b, float x)
{
	float ft = x * 3.1415927f;
	float f = (1.0f - cos(ft)) * 0.5f;

	return  a * ( 1 - f ) + b * f;
}

float Noise(int x, int y)
{
	int n = x + y * 57;
	n = ( n << 13 ) ^ n;
	return ( 1.f - ( ( n * ( n * n * 15731 + 789221 ) + 1376312589 ) & 0x7fffffff ) / 1073741824.0 );    
}

float SmoothedNoise1(float x, float y)
{
	float corners = ( Noise(x-1, y-1)+Noise(x+1, y-1)+Noise(x-1, y+1)+Noise(x+1, y+1) ) / 16.0f;
	float sides   = ( Noise(x-1, y)  +Noise(x+1, y)  +Noise(x, y-1)  +Noise(x, y+1) ) /  8.0f;
	float center  =  Noise(x, y) / 4.0f;
	return corners + sides + center;
}

float InterpolatedNoise1(float x, float y)
{
	int integer_X    = (int)floor(x);
	float fractional_X = fabs (x - integer_X);

	int integer_Y    = (int)floor(y);
	float fractional_Y = fabs(y - integer_Y);

	float v1 = SmoothedNoise1(integer_X,     integer_Y);
	float v2 = SmoothedNoise1(integer_X + 1, integer_Y);
	float v3 = SmoothedNoise1(integer_X,     integer_Y + 1);
	float v4 = SmoothedNoise1(integer_X + 1, integer_Y + 1);

	float i1 = Cosine_Interpolate(v1 , v2 , fractional_X);
	float i2 = Cosine_Interpolate(v3 , v4 , fractional_X);

	return Cosine_Interpolate(i1 , i2 , fractional_Y);
}

float GetPerlin( float x, float y)
{
	float total = 0.0f;
	float p = /*persistence*/0.5f;
	float freq = 2.0f;
	int n = /*Number_Of_Octaves*/8- 1;

	for( int i = 0; i < n; ++i )
	{
		float frequency = pow(freq, (float)i);
		float amplitude = pow(p, (float)i);

		total = total + InterpolatedNoise1(x * frequency, y * frequency) * amplitude;
	}

	return total;
}

///////////////////////////////////////////////////////////////////////////////

__kernel void
springjob( __global float4 * output, 
		  __global float4 * input,
		  float2 basePos,
		  float spread,
		  float speed,
		  float sample_coeff,
		  float circle_coeff,
		  float dt,
		  float totalTime )
{
	uint index = get_global_id(0);
    uint numParticles = get_global_size(0);
	float indexNormalized = (float)index/numParticles;

	float4 vertData = input[index];
	float2 pos = vertData.xy;
	float angle = mix(0, spread*3.14159f, 
					GetPerlin(pos.x * sample_coeff,	pos.y * sample_coeff)*0.5f+0.5f+indexNormalized*circle_coeff);

	float2 dir;
	dir.x = cos(angle);
	dir.y = sin(angle);

	float2 newPos = input[index].xy + speed * dir * dt;
	output[index].xy = newPos;
	output[index].zw = dir * 0.5f + 0.5f;
	//output[index].zw = nextVel;
}