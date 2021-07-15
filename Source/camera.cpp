#include "camera.h"
#include "EffekseerForDXLib.h"

Camera::Camera(SceneBase * scene)
{
	position = VGet(100.f, 150.0f, -200.0f);
	target = VGet(0, 100, 0);
}

Camera::~Camera()
{
}

void Camera::Update()
{
}

void Camera::Draw()
{
	SetCameraPositionAndTarget_UpVecY(position, target);
	// DXライブラリのカメラとEffekseerのカメラを同期する。
	Effekseer_Sync3DSetting();
}
