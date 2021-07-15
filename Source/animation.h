#pragma once

class Animation {
public:
	Animation(int _hModel);
	~Animation();
	void Update();
	void Start(int _hAnim, bool loop); //�A�j���[�V����
	bool IsStart();
	bool IsFinish();
	float GetFrameTime() { return frameTime; }
	int GethAnim() { return hAnim; }
	void ResetTime() { frameTime = 0.0f; }
private:
	int hModel; //�n���ꂽ���̂�ۑ�
	int hAnim;
	int prevhAnim;
	
	bool isLoop;
	float frameTime;
	float frameTimeMax;
};