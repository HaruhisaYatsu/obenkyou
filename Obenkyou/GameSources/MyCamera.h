#pragma once


#include "stdafx.h"

namespace basecross {

	class MyCamera : public Camera {

	public:

		explicit MyCamera();

		virtual ~MyCamera();

		shared_ptr<GameObject> GetTargetObject() const;

		void SetTargetObject(const shared_ptr<GameObject>& Obj);

		float GetToTargetLerp() const;

		void SetToTargetLerp(float f);

		//�A�N�Z�T
		//shared_ptr< StateMachine<MyCamera> > GetStateMachine() const {
		//	return m_StateMachine;
		//}
		//virtual void OnCreate();
		virtual void OnUpdate();

		////SetEye�ύX�p�ϐ�
		//float ChangeSetEyeX = 0.0f;
		//float ChangeSetEyeY = 0.0f;
		////���̂��߂̃Z�b�^�[�֐�
		//void setChangeSetEyeX(float ChangeRange) {
		//	ChangeSetEyeX = ChangeRange;
		//}
		//void setChangeSetEyeY(float ChangeRange) {
		//	ChangeSetEyeY = ChangeRange;
		//}

	private:
		// pImpl�C�f�B�I��
		struct Impl;
		unique_ptr<Impl> pImpl;

	};

}