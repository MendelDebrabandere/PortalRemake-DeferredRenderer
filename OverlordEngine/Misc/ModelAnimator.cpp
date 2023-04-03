#include "stdafx.h"
#include "ModelAnimator.h"

ModelAnimator::ModelAnimator(MeshFilter* pMeshFilter):
	m_pMeshFilter{pMeshFilter}
{
	SetAnimation(0);
}

void ModelAnimator::Update(const SceneContext& sceneContext)
{
	//TODO_W6_();

	//We only update the transforms if the animation is running and the clip is set
	if (m_IsPlaying && m_ClipSet)
	{
		//1. 
		//Calculate the passedTicks (see the lab document)
		auto passedTicks = sceneContext.pGameTime->GetElapsed() * m_CurrentClip.ticksPerSecond * m_AnimationSpeed;
		//Make sure that the passedTicks stay between the m_CurrentClip.Duration bounds (fmod)
		if (passedTicks > m_CurrentClip.duration)
			passedTicks = fmodf(passedTicks, m_CurrentClip.duration);

		//2. 
		//IF m_Reversed is true
		if (m_Reversed)
		{
			//	Subtract passedTicks from m_TickCount
			m_TickCount -= passedTicks;
			//	If m_TickCount is smaller than zero, add m_CurrentClip.Duration to m_TickCount
			if (m_TickCount < 0)
				m_TickCount += m_CurrentClip.duration;
		}
		//ELSE
		else
		{
			//	Add passedTicks to m_TickCount
			m_TickCount += passedTicks;
			//	if m_TickCount is bigger than the clip duration, subtract the duration from m_TickCount
			if (m_TickCount > m_CurrentClip.duration)
				m_TickCount -= m_CurrentClip.duration;
		}

		//3.
		//Find the enclosing keys
		AnimationKey keyA{}, keyB{};
		//Iterate all the keys of the clip and find the following keys:
		//keyA > Closest Key with Tick before/smaller than m_TickCount
		//keyB > Closest Key with Tick after/bigger than m_TickCount
		for (int i{}; i < m_CurrentClip.keys.size(); ++i)
		{
			const auto& key{ m_CurrentClip.keys[i] };

			if (key.tick > m_TickCount)
			{
				keyB = key;
				keyA = m_CurrentClip.keys[i - 1];
				break;
			}
		}

		//4.
		//Interpolate between keys
		//Figure out the BlendFactor (See lab document)
		const float blendFactor{ 1 - (keyB.tick - m_TickCount) / (m_CurrentClip.duration / (m_CurrentClip.keys.size() - 1)) };

		//Clear the m_Transforms vector
		//m_Transforms.clear();
		//FOR every boneTransform in a key (So for every bone)
		for (int i{}; i < m_pMeshFilter->GetBoneCount(); ++i)
		{
			//	Retrieve the transform from keyA (transformA)
			const auto& transformA{ keyA.boneTransforms[i] };
			// 	Retrieve the transform from keyB (transformB)
			const auto& transformB{ keyB.boneTransforms[i] };

			//	Decompose both transforms
			XMVECTOR translationA;
			XMVECTOR rotationA;
			XMVECTOR scaleA;
			XMMatrixDecompose(&scaleA, &rotationA, &translationA, XMLoadFloat4x4(&transformA));

			XMVECTOR translationB;
			XMVECTOR rotationB;
			XMVECTOR scaleB;
			XMMatrixDecompose(&scaleB, &rotationB, &translationB, XMLoadFloat4x4(&transformB));


			//	Lerp between all the transformations (Position, Scale, Rotation)
			const XMVECTOR scaleLerp{ XMVectorLerp(scaleA, scaleB, blendFactor) };
			const XMVECTOR rotationSlerp{ XMQuaternionSlerp(rotationA, rotationB, blendFactor) };
			const XMVECTOR translationLerp{ XMVectorLerp(translationA, translationB, blendFactor) };

			//	Compose a transformation matrix with the lerp-results
			const XMMATRIX transform{ XMMatrixScalingFromVector(scaleLerp) * XMMatrixRotationQuaternion(rotationSlerp) * XMMatrixTranslationFromVector(translationLerp) };

			//	Add the resulting matrix to the m_Transforms vector
			XMStoreFloat4x4(&m_Transforms[i], transform);

		}
	}
}

void ModelAnimator::SetAnimation(const std::wstring& clipName)
{
	//TODO_W6_();
	//Set m_ClipSet to false
	m_ClipSet = false;
	//Iterate the m_AnimationClips vector and search for an AnimationClip with the given name (clipName)
	for (const auto& animClip : m_pMeshFilter->GetAnimationClips())
	{
		//If found,
		//	Call SetAnimation(Animation Clip) with the found clip
		if (animClip.name == clipName)
		{
			SetAnimation(animClip);
			return;
		}
	}
	//Else

	//	Call Reset
	Reset();
	//	Log a warning with an appropriate message
	Logger::LogWarning(L"No animationClip found with the given name");
}

void ModelAnimator::SetAnimation(UINT clipNumber)
{
	//TODO_W6_();
	//Set m_ClipSet to false
	m_ClipSet = false;
	//Check if clipNumber is smaller than the actual m_AnimationClips vector size
	//If not,
	const auto& animationClips{ m_pMeshFilter->GetAnimationClips() };
	if (clipNumber < animationClips.size() && clipNumber >= 0)
	{
		//	Retrieve the AnimationClip from the m_AnimationClips vector based on the given clipNumber
		//	Call SetAnimation(AnimationClip clip)
		SetAnimation(m_pMeshFilter->GetAnimationClips()[clipNumber]);
		return;
	}

	//	Call Reset
	Reset();
	//	Log a warning with an appropriate message
	Logger::LogWarning(L"clipNumber is not smaller than the actual m_AnimationClips vector size");
}

void ModelAnimator::SetAnimation(const AnimationClip& clip)
{
	//TODO_W6_();
	//Set m_ClipSet to true
	m_ClipSet = true;
	//Set m_CurrentClip
	m_CurrentClip = clip;
	//Call Reset(false)
	Reset(false);
}

void ModelAnimator::Reset(bool pause)
{
	//TODO_W6_();
	//If pause is true, set m_IsPlaying to false
	if (pause)
		m_IsPlaying = false;
	//Set m_TickCount to zero
	m_TickCount = 0;
	//Set m_AnimationSpeed to 1.0f
	m_AnimationSpeed = 1.0f;
	//If m_ClipSet is true
	if (m_ClipSet)
	{
		//	Retrieve the BoneTransform from the first Key from the current clip (m_CurrentClip)
		const auto& boneTransforms = m_CurrentClip.keys[0].boneTransforms;
		//	Refill the m_Transforms vector with the new BoneTransforms (have a look at vector::assign)
		m_Transforms.assign(boneTransforms.begin(), boneTransforms.end());
		return;
	}
	//Else

	//	Create an IdentityMatrix
	const XMFLOAT4X4 IMatrix{
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1
	};
	//	Refill the m_Transforms vector with this IdenityMatrix (Amount = BoneCount) (have a look at vector::assign)
	m_Transforms.assign(m_pMeshFilter->GetBoneCount(), IMatrix);
}