/*!
@file GameStage.h
@brief �Q�[���X�e�[�W
*/

#pragma once
#include "stdafx.h"

namespace basecross {

	//--------------------------------------------------------------------------------------
	//	�Q�[���X�e�[�W�N���X
	//--------------------------------------------------------------------------------------
	class GameStage : public Stage {
		//�r���[�̍쐬
		void CreateViewLight();
		//�v���[�g�̍쐬
		void CreatePlate();
		//�v���C���[�̍쐬
		void CreatePlayer();
		//Box�̍쐬
		void CreateBox();
		//Sprite�̍쐬
		void CreateSprite();
		//Spark�̍쐬
		void CreateSpark();

	public:
		//�\�z�Ɣj��
		GameStage() :Stage() {}
		virtual ~GameStage() {}

		virtual void OnUpdate()override;
		//������
		virtual void OnCreate()override;
	};


}
//end basecross

