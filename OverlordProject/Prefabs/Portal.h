#pragma once
enum class PortalType
{
	Orange = 0,
	Blue = 1
};

class Portal final : public GameObject
{
public:
	Portal(PortalType type);
	~Portal() override = default;

	Portal(const Portal& other) = delete;
	Portal(Portal&& other) noexcept = delete;
	Portal& operator=(const Portal& other) = delete;
	Portal& operator=(Portal&& other) noexcept = delete;

protected:
	void Initialize(const SceneContext&) override;
	void Update(const SceneContext&) override;

private:
	PortalType m_Type{};

	MeshDrawComponent* m_Plane{};


};
