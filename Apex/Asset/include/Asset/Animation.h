#pragma once
#include "Containers/AxArray.h"
#include "Math/Quaternion.h"
#include "Math/Vector3.h"


using apex::math::Vector3;
using apex::math::Quat;

namespace apex {
namespace asset {

	struct AnimationKeyframe
	{
		f32 time;
		Vector3 position;
		Quat rotation;
	};

	struct AnimationTrack
	{
		u32 boneId;
		u32 keyframeCount;
		AnimationKeyframe* keyframes;
	};

	struct AnimationClip
	{
		f32 duration;
		f32 fps;
		u32 trackCount;
		AnimationTrack *tracks;
	};

}
}
