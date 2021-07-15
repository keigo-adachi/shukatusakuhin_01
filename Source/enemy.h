#pragma once
#include "../Library//gameObject.h"
#include "animation.h"

class Enemy :public GameObject {
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
		MAX
	};

	Enemy(SceneBase*scene);
	~Enemy();
	void Start()override;
	void Update()override;
	void Draw()override;

	VECTOR GetPosition() { return Position; }

	/// <summary>
	/// �J�v�Z���Ƌ��̂ł̔���
	/// </summary>
	bool HitCollision(VECTOR _cpos,float _crad,bool _downAttack);

	/// <summary>
	/// �~���Ɖ~���̓����蔻��
	/// </summary>
	VECTOR CollisionPlayer(VECTOR center, float r, VECTOR vel);


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
		STATE_JUMPGUARD,		// �K�[�h�W�����v��
		STATE_WASATTACK,		// �U�����󂯂�
		STATE_DOWN,				// �_�E������
		STATE_DOWN_LOOP,		// �_�E�����[�v
		STATE_STANDUP			// �����オ��
	};
	STATE status; // ���݂̃X�e�[�^�X
	STATE beforestatus; // �ЂƂO�̃X�e�[�^�X

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
	void DownLoop();
	void StandUp();
	void HitField();
	void SetCamera();
	void SetCharacterRotationY();

	Animation* animation;

	int hModel; // ���f���̃n���h���[

	int MoveAnimFrameIndex;
	VECTOR Position; // ���[���h���W
	VECTOR TempPosition; // �ꎞ�I�ȃ��[���h���W
	VECTOR NowPosition; // ���[�J�����W���݂̈ʒu
	VECTOR PrevPosition; // ���[�J�����W���_���W
	VECTOR Rotation; // ��]�l
	VECTOR Vel;

	float Angle; // �J�����̉�]�l(��)
	float Angle2; // �J�����̉�]�l(�c)

	bool isJump; // �W�����v���Ă����true

	float jumpPower; // �W�����v��

	VECTOR hit; // Ray�ƒn�ʂƂ̐ڒn

	int ComboNum; //�U����

	VECTOR p1, p2; //�����̎n�_�I�_

	float rad; //���a

};