class PongScene final : public GameScene
{
public:
	PongScene() : GameScene(L"PongScene") {};
	~PongScene() override = default;

	PongScene(const PongScene& other) = delete;
	PongScene(PongScene&& other) noexcept = delete;
	PongScene& operator=(const PongScene& other) = delete;
	PongScene& operator=(PongScene&& other) noexcept = delete;

protected:
	void Initialize() override;
	void Update() override;
	//void Draw() override;
	void OnGUI() override;

private:
	GameObject* m_pPongLeft{ nullptr };
	GameObject* m_pPongRight{ nullptr };
	GameObject* m_pBall{ nullptr };

	bool m_IsBallMoving{ false };

	GameObject* m_pLeftTrigger{ nullptr };
	GameObject* m_pRightTrigger{ nullptr };

	/////////////////////////////////////////////////////////
	const XMFLOAT3 pongRectSize{ 3.f, 2.f, 8.f };
	const XMFLOAT4 pongColor{ Colors::White };

	const float ballRadius{ 2.f };
	const XMFLOAT4 ballColor{ Colors::Red };

	const XMFLOAT2 fieldSize{ 35, 23 };

	const float staticDensity{ 100000 };
};