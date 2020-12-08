
#ifndef _NES_REWIND_H_
#define _NES_REWIND_H_

/*
#define NES_REWIND_KEY_FRAME_NUM 5
#define NES_REWIND_FRAME_INT 30
*/


typedef struct  {
	char InputData[4];
}NES_Rewind_InputData;



class NES_Rewind_FrameData {
public:
	NES_Rewind_FrameData(unsigned char *p, int bf){
		m_pState = p;
		m_nInput = 0;
		m_Bframe = bf;

		m_InputData = (NES_Rewind_InputData *)malloc(m_Bframe * sizeof(NES_Rewind_InputData));
	}

	~NES_Rewind_FrameData(){
		if(m_pState){
			free(m_pState);
			m_pState = NULL;
		}
		if(m_InputData){
			free(m_InputData);
			m_InputData = NULL;
		}
	}

	unsigned char *GetStateData(){
		return m_pState;
	}

	NES_Rewind_InputData *GetInputData(uint32 n){
		return &m_InputData[n];
	}

	uint32 GetInputDataNum(){
		return m_nInput;
	}

	void Rewind(uint32 n){
		if(m_nInput >= n)
			m_nInput -= n;
	}

	int AddInputData(unsigned char *p){
		if(m_nInput >= (m_Bframe-1)){
			return 1;
		}
		m_InputData[m_nInput].InputData[0] = p[0];
		m_InputData[m_nInput].InputData[1] = p[1];
		m_InputData[m_nInput].InputData[2] = p[2];
		m_InputData[m_nInput].InputData[3] = p[3];
		m_nInput++;
		return 0;
	}

	int IsFull(){
		if(m_nInput >= (m_Bframe-1)){
			return 1;
		}
		return 0;
	}


private:
	unsigned char *m_pState;
	uint32 m_nInput;
	uint32 m_Bframe;
//	unsigned char *m_InputData[(NES_REWIND_FRAME_INT-1)][4];
	NES_Rewind_InputData *m_InputData;
};



class NES_Rewind {
public:
	NES_Rewind(int k, int b){
		RewindStateNum = 0;
		RewindDataCur = NULL;
		RewindMaxFrame = k;
		RewindMaxBFrame= b;

		RewindData = (NES_Rewind_FrameData **)malloc(RewindMaxFrame * sizeof(NES_Rewind_FrameData *));
		for(uint32 i=0; i<RewindMaxFrame; i++){
			RewindData[i] = NULL;
		}
	}

	~NES_Rewind(){
		for(uint32 i=0; i<RewindMaxFrame; i++){
			if(RewindData[i]){
				delete RewindData[i];
				RewindData[i] = NULL;
			}
		}
	}

	int NeedStateData(){
		if(RewindDataCur == NULL || RewindDataCur->IsFull())
			return 1;
		return 0;
	}

	int AddStateData(unsigned char *p){
		RewindDataCur = new NES_Rewind_FrameData(p, RewindMaxBFrame);
		if(!RewindDataCur)
			return 1;
		if(RewindStateNum >= RewindMaxFrame){
			delete RewindData[0];
			for(uint32 i=0; i<(RewindMaxFrame-1); i++){
				RewindData[i] = RewindData[i+1];
			}
			RewindData[(RewindMaxFrame-1)] = RewindDataCur;
		}
		else {
			RewindData[RewindStateNum] = RewindDataCur;
			RewindStateNum++;
		}
		return 0;
	}

	int AddFrameData(unsigned char *p){
		if(RewindDataCur){
			RewindDataCur->AddInputData(p);
		}
		return 0;
	}

	int Rewind(uint32 n){
		uint32 cd;
		if(!RewindDataCur)
			return 1;
		cd = RewindDataCur->GetInputDataNum();
		if(n > cd){
			if(RewindStateNum >= 2){
				RewindStateNum--;
				delete RewindData[RewindStateNum];
				RewindData[RewindStateNum] = NULL;
				RewindDataCur = RewindData[RewindStateNum-1];

//		LOG("RewindDataCur" << (int)RewindDataCur << ", RewindStateNum" << RewindStateNum << endl);
				RewindDataCur->Rewind(n-(cd+1));
			}
		}
		else {
			RewindDataCur->Rewind(n);
		}
		return 0;
	}

	NES_Rewind_FrameData *GetCur(){
		return RewindDataCur;
	}

private:
	uint32 RewindMaxFrame;
	uint32 RewindMaxBFrame;
	uint32 RewindStateNum;	// stateêî
	NES_Rewind_FrameData **RewindData;
	NES_Rewind_FrameData *RewindDataCur;

};



#endif



