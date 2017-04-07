/*!
@file GameStage.cpp
@brief �Q�[���X�e�[�W����
*/

#include "stdafx.h"
#include "Project.h"

namespace basecross {

	//--------------------------------------------------------------------------------------
	//	�Q�[���X�e�[�W�N���X����
	//--------------------------------------------------------------------------------------

	//�r���[�ƃ��C�g�̍쐬
	void GameStage::CreateViewLight() {
		auto PtrView = CreateView<SingleView>();
		//�r���[�̃J�����̐ݒ�
		auto PtrLookAtCamera = ObjectFactory::Create<MyCamera>();
		PtrView->SetCamera(PtrLookAtCamera);
		PtrLookAtCamera->SetEye(Vector3(0.0f, 13.0f, -5.0f));
		PtrLookAtCamera->SetAt(Vector3(0.0f, -10.0f, 0.0f));
		//�}���`���C�g�̍쐬
		auto PtrMultiLight = CreateLight<MultiLight>();
		//�f�t�H���g�̃��C�e�B���O���w��
		PtrMultiLight->SetDefaultLighting();
	}


	//�v���[�g�̍쐬
	void GameStage::CreatePlate() {
		//�X�e�[�W�ւ̃Q�[���I�u�W�F�N�g�̒ǉ�
		auto Ptr = AddGameObject<GameObject>();
		auto PtrTrans = Ptr->GetComponent<Transform>();
		Quaternion Qt(Vector3(1.0f, 0, 0), XM_PIDIV2);
		Qt.RotationRollPitchYawFromVector(Vector3(XM_PIDIV2, 0, 0));
		PtrTrans->SetScale(50.0f, 50.0f, 1.0f);
		PtrTrans->SetQuaternion(Qt);
		PtrTrans->SetPosition(0.0f, 0.0f, 0.0f);

		auto ColPtr = Ptr->AddComponent<CollisionRect>();
		//�`��R���|�[�l���g�̒ǉ�
		auto DrawComp = Ptr->AddComponent<BcPNTStaticDraw>();
		//�`��R���|�[�l���g�Ɍ`��i���b�V���j��ݒ�
		DrawComp->SetMeshResource(L"DEFAULT_SQUARE");
		DrawComp->SetFogEnabled(true);
		//�����ɉe���f�肱�ނ悤�ɂ���
		DrawComp->SetOwnShadowActive(true);

		//�`��R���|�[�l���g�e�N�X�`���̐ݒ�
		DrawComp->SetTextureResource(L"SKY_TX");
	}
	
	//�v���C���[�̍쐬
	void GameStage::CreatePlayer() {
		//�v���[���[�̍쐬
		auto PlayerPtr = AddGameObject<Player>();
		//�V�F�A�z��Ƀv���C���[��ǉ�
		SetSharedGameObject(L"Player", PlayerPtr);
	}
	//Box�̍쐬

	void GameStage::CreateSpark() {
		auto MultiSparkPtr = AddGameObject<MultiSpark>();

		SetSharedGameObject(L"MultiSpark", MultiSparkPtr);
	
	
	}




	void GameStage::CreateBox() {
		
		
		vector< vector<Vector3> > Vec = {
			{
				Vector3(1.0f, 1.0f, 1.0f),	//�X�P�[��
				Vector3(0.0f, 0.0f, 0.0f),	//��[�ā[�����
				Vector3(5.0f, 0.5f, 0.0f)	//�|�W�V����
			},
			{
				Vector3(1.0f, 1.0f, 1.0f),	//�X�P�[��
				Vector3(0.0f, 0.0f, 0.0f),	//��[�ā[�����
				Vector3(6.0f, 0.5f, 1.0f)	//�|�W�V����
			},
			{
				Vector3(1.0f, 1.0f, 1.0f),	//�X�P�[��
				Vector3(0.0f, 0.0f, 0.0f),	//��[�ā[�����
				Vector3(4.0f, 0.5f, -1.0f)	//�|�W�V����
			},
			{
				Vector3(1.0f, 1.0f, 1.0f),	//�X�P�[��
				Vector3(0.0f, 0.0f, 0.0f),	//��[�ā[�����
				Vector3(4.0f, 0.5f, 1.0f)	//�|�W�V����
			},
			{
				Vector3(1.0f, 1.0f, 1.0f),	//�X�P�[��
				Vector3(0.0f, 0.0f, 0.0f),	//��[�ā[�����
				Vector3(6.0f, 0.5f, -1.0f)	//�|�W�V����
			},
		};
		//�I�u�W�F�N�g�̍쐬
		for (auto v : Vec) {
			AddGameObject<FixedBox>(v[0], v[1], v[2]);
		}
	}
	
	void GameStage::CreateSprite() {
		AddGameObject<WallSprite>(
			L"WALL_TX",
			true,
			Vector2(200.0f, 200.0f),
			Vector2(440.0f, -200.0f));
	}


	void GameStage::OnCreate() {
		try {
			//�r���[�ƃ��C�g�̍쐬
			CreateViewLight();
			//�v���[�g�̍쐬
			CreatePlate();
			//�v���[���[�̍쐬
			CreatePlayer();
			//Box�̍쐬
			//CreateBox();
			//Sprite�̍쐬
			CreateSprite();
			CreateSpark();
		}
		catch (...) {
			throw;
		}
		
	}
	void GameStage::OnUpdate() {
		auto Mouse = App::GetApp()->GetInputDevice().GetKeyState();
		auto ScenePtr = App::GetApp()->GetScene<Scene>();
		if (Mouse.m_bPressedKeyTbl[VK_RBUTTON]) {
			PostEvent(0.0f, GetThis<GameStage>(), ScenePtr, L"Tostage_title");
		}
	}
}
//end basecross
