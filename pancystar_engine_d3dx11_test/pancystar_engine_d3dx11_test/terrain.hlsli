float count_terrain_height(float depth_sample)
{
	return (2.0f*depth_sample - 1.0f) * 300.0f;
}