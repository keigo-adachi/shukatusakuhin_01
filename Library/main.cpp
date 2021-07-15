#include <DxLib.h>
#include <EffekseerForDXLib.h>
#include "App.h"
#include "../Source/config.h"

#define CoGVersion (2.0)

// �v���O������ WinMain ����n�܂�܂�
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	SetGraphMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32);
	SetOutApplicationLogValidFlag(FALSE); // ���O���o���Ȃ�
	SetDrawScreen(DX_SCREEN_BACK);

	// DirectX11���g�p����悤�ɂ���B(DirectX9���A�ꕔ�@�\�s��)
	// Effekseer���g�p����ɂ͕K���ݒ肷��B
	SetUseDirect3DVersion(DX_DIRECT3D_11);

	SetMainWindowText(WINDOW_NAME);
	SetWindowSizeExtendRate(WINDOW_EXTEND);
	ChangeWindowMode(WINDOW_MODE); // Window���[�h�̏ꍇ

	if (DxLib_Init() == -1)		// �c�w���C�u��������������
	{
		return -1;			// �G���[���N�����璼���ɏI��
	}

	// Effekseer������������B
	// �����ɂ͉�ʂɕ\������ő�p�[�e�B�N������ݒ肷��B
	if (Effekseer_Init(8000) == -1)
	{
		DxLib_End();
		return -1;
	}

	// �t���X�N���[���E�C���h�E�̐؂�ւ��Ń��\�[�X��������̂�h���B
	// Effekseer���g�p����ꍇ�͕K���ݒ肷��B
	SetChangeScreenModeGraphicsSystemResetFlag(FALSE);

	// DX���C�u�����̃f�o�C�X���X�g�������̃R�[���o�b�N��ݒ肷��B
	// �E�C���h�E�ƃt���X�N���[���̐؂�ւ�����������ꍇ�͕K�����s����B
	// �������ADirectX11���g�p����ꍇ�͎��s����K�v�͂Ȃ��B
	Effekseer_SetGraphicsDeviceLostCallbackFunctions();


	SetAlwaysRunFlag(TRUE);
	SetUseZBuffer3D(TRUE);
	SetWriteZBuffer3D(TRUE);

	AppInit();
	while (true) {
		AppUpdate();
		
		// Effekseer�ɂ��Đ����̃G�t�F�N�g���X�V����B
		UpdateEffekseer3D();

		ClearDrawScreen();
		AppDraw();
		// Effekseer�ɂ��Đ����̃G�t�F�N�g��`�悷��B
		if (ProcessMessage() == -1 || CheckHitKey(KEY_INPUT_ESCAPE) != 0 )
			break;
		ScreenFlip();
		WaitTimer(16);


	}
	AppRelease();
	

	// Effekseer���I������B
	Effkseer_End();

	DxLib_End();				// �c�w���C�u�����g�p�̏I������

	return 0;				// �\�t�g�̏I�� 
} 