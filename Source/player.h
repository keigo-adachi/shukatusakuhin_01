#pragma once
#include "../Library//gameObject.h"
#include "animation.h"
#include "camera.h"
#include "enemy.h"
#include "effectManager.h"
class Player :public GameObject{
public:
	//�A�j���[�V����
	enum ANIM_ID {
		NEUTRAL = 0,	//�ҋ@
		WALK_START,		//�����o��
		WALK,			//����
		WALK_END,		//�����~�܂�
		RUN_START,		//����o��
		RUN,			//����
		RUNEND,			//����~�܂�
		TACKLE,			//�ːi
		DOWN,			//������΂���
		STANDUP,		//�����オ��
		ATTACK_1,		//�ʏ�U��(1���)
		ATTACK_2,		//�ʏ�U��(2���)
		ATTACK_3,		//�ʏ�U��(3���)
		GUARD,			//�K�[�h
		GUARD_SUCCES,	//�K�[�h����
		KNOCKBACK,		//�̂�����
		JUMP_START,		//�W�����v�n��
		JUMP,			//�W�����v
		JUMP_END,		//�W�����v�I���
		DODGE,			//���
		DOWN_LOOP,		//�|��
	};

	enum ATTACK{
		NORMAL_1,
		NORMAL_2,
		NORMAL_3,
		TACKLE_1,
		MAX
	};
	
	Player(SceneBase*scene);
	~Player();
	void Start()override;
	void Update()override;
	void Draw()override;

	VECTOR GetPosition() { return Position; }; //�v���C���[�̍��W���擾
	
private:
	//�X�e�[�^�X
	enum STATE
	{
		STATE_NEUTRAL = 0,		// �ҋ@
		STATE_MOVE,				// �s�������߂�
		STATE_WALK,				// ����
		STATE_RUN,				// ����
		STATE_JUMP,				// �W�����v
		STATE_COMBOATTACK,		// �A���U��
		STATE_DASHATTACK,		// �_�b�V���U��
		STATE_JUMPCOMBOATTACK,  // �W�����v�A���U��
		STATE_DODGE,			// ���
		STATE_GUARD,			// �K�[�h
		STATE_JUMPGUARD,		// �K�[�h�i�W�����v���j
		STATE_WASATTACK,		// �U�����󂯂�
		STATE_DOWN,				// �_�E������
	};
	STATE status; // ���݂̃X�e�[�^�X
	STATE beforestatus; // �ЂƂO�̃X�e�[�^�X

	ATTACK ComboNum;

	// �X�e�[�^�X�ɂ�鏈��
	void Move();
	void Walk();
	void Run();
	void Jump();
	void ComboAttack();
	void DashAttack();
	void JumpComboAttack();
	void Dodge();
	void Guard();
	void JumpGuard();
	void WasAttack();
	void Down();

	void HitField();
	void SetCamera();
	void SetCharacterRotationY();

	Animation* animation;
	Camera*Cm;
	Enemy* enemy;
	EffectManager* effectManager;

	int hModel; // ���f���̃n���h���[

	int MoveAnimFrameIndex;
	VECTOR Position; // ���[���h���W
	VECTOR TempPosition; // �ꎞ�I�ȃ��[���h���W
	VECTOR NowPosition; // ���[�J�����W���݂̈ʒu
	VECTOR PrevPosition; // ���[�J�����W���_���W
	VECTOR Rotation; // ��]�l
	VECTOR Vel;	//�ړ���

	float YawAngle; // �J�����̉�]�l(��)
	float PitchAngle; // �J�����̉�]�l(�c)

	VECTOR CPos; // �J�������W

	bool isJump; // �W�����v���Ă����true

	float jumpPower; // �W�����v��

	VECTOR hit; // Ray�ƒn�ʂƂ̐ڒn

	//�U�����̓����蔻��
	struct ATTACKCOLLISION
	{
		VECTOR Position;	//���W
		float Radius;		//���a
		int FrameIndex;		//�A�^�b�`����t���[���̔ԍ�
		bool isEnable;		//�L�����ǂ���
		bool DownAttack;	//�_�E������U�����ǂ���
	};
	ATTACKCOLLISION  Collision[Player::ATTACK::MAX]; //���i���E�j0,1�A��(�E�j2,���� 3
};