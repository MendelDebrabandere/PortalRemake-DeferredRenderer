#pragma once
//Resharper Disable All

class PostBrightness : public PostProcessingMaterial
{
public:
	PostBrightness();
	~PostBrightness() override = default;
	PostBrightness(const PostBrightness& other) = delete;
	PostBrightness(PostBrightness&& other) noexcept = delete;
	PostBrightness& operator=(const PostBrightness& other) = delete;
	PostBrightness& operator=(PostBrightness&& other) noexcept = delete;

protected:
	void Initialize(const GameContext& /*gameContext*/) override {}
};

