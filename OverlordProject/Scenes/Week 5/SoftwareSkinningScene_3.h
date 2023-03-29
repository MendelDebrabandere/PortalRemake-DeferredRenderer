#pragma once

class BoneObject;

class SoftwareSkinningScene_3 : public GameScene
{
public:
	SoftwareSkinningScene_3() : GameScene(L"SoftwareSkinningScene_3") {};
	~SoftwareSkinningScene_3() override = default;

	SoftwareSkinningScene_3(const SoftwareSkinningScene_3& other) = delete;
	SoftwareSkinningScene_3(SoftwareSkinningScene_3&& other) noexcept = delete;
	SoftwareSkinningScene_3& operator=(const SoftwareSkinningScene_3& other) = delete;
	SoftwareSkinningScene_3& operator=(SoftwareSkinningScene_3&& other) noexcept = delete;

protected:
	void Initialize() override;
	void Update() override;
	void OnGUI() override;

private:
	BoneObject* m_pBone0{}, * m_pBone1{};

	float m_BoneRotation{}, m_BoneRotation0{}, m_BoneRotation1{};
	int m_RotationSign{ 1 };

	bool m_ManualMode{ false };

	struct VertexSoftwareSkinned
	{
		VertexSoftwareSkinned(XMFLOAT3 position, XMFLOAT3 normal, XMFLOAT4 color, float blendRate) :
			transformedVertex{ position, normal, color },
			originalVertex{ position, normal, color },
			blendWeight0{ blendRate },
			blendWeight1{ 1 - blendRate } {}

		VertexPosNormCol transformedVertex{};
		VertexPosNormCol originalVertex{};

		float blendWeight0{};
		float blendWeight1{};
	};

	void InitializeVertices(float length);

	MeshDrawComponent* m_pMeshDrawer{};
	std::vector<VertexSoftwareSkinned> m_SkinnedVertices{};
};

