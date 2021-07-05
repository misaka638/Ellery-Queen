class TextSimilarity
{
public:
	typedef std::unordered_map<std::string, double> WordFrep;//��Ƶ
	typedef std::unordered_set<std::string> WordSet;//�ʱ�
	typedef std::pair<std::string, double> PSI;//��ֵ��
	TextSimilarity(const char* dictPach);//���캯��
	TextSimilarity(Config& cfg);
	void printStopWordSet();//��ӡͣ�ôʱ�
	void printWordFrep(const WordFrep& wordFreq);//��ӡ��Ƶ
	double getTextSimilarity(const char* file1, const char* file2);//�ṩ��ȡ�ı����ƶȵĽӿ�

//private: 
	//��ȡͣ�ôʱ�
	void getStopWordSet(const char* file);
	//��ȡIDF�����ĵ��ʣ�����ʾÿ������Ҫ�Ե�Ȩ��
	void getIDF(const char* file);
	//ͳ�ƴ�Ƶ: word   num(map�������������ģ�����������ģ�
	//��Ŀ������ҪValue���򣬵�map��֧�֣�����ʹ�������map��Ч�ʽϸ�
	void getWordFrep(const char* file, WordFrep& wordFreq);
	//ͳ����Դ�Ƶ�����ʴ�Ƶ/���µ�������
	void getNormalizedWordFrep(WordFrep& wordFreq);
	//ͳ�Ƽ���IDFȨ�صĴ�Ƶ
	void getTfIdf(WordFrep& wordFreq, WordFrep& outTfIdf);
	//GBKתUTF8
	std::string GBK2UTF8(const std::string& gbk);
	//UTF8תGBK
	std::string UTF82GBK(const std::string& utf8);
	//����Valueֵ��������
	void sortReverseByValue(const WordFrep& wordFreq, std::vector<PSI>& outSortedWordFreq);
	//����ͳһ�Ĵʱ�
	void getWordCode(std::vector<PSI>& inSortWordFreq, WordSet& wordCode);
	//���ݴʱ�������Ƶ����
	void getVector(WordSet& wordCode, WordFrep& wordFreq, std::vector<double>& outVec);
	//�����ı����ƶ�
	double getCosine(std::vector<double>& vec1, std::vector<double>& vec2);

	//�ִʶ����Ա
	std::string DICT_PATH;//�ʵ�·��
	cppjieba::Jieba _jieba;//�ִʶ���


	//ͣ�ôʱ�ֻ��Ҫ���ͣ�ôʣ���string���ַ��������洢����һ����ϣ��
	WordSet _stopWordSet;
	int _maxWordNum;
	WordFrep _Idf;
};
//���캯����
TextSimilarity::TextSimilarity(const char* dictPach)
	:DICT_PATH(dictPach)
	, _jieba(
		DICT_PATH + "/jieba.dict.utf8"
		, DICT_PATH + "/hmm_model.utf8"
		, DICT_PATH + "/user.dict.utf8"
		, DICT_PATH + "/idf.utf8"
		, DICT_PATH + "/stop_words.utf8"
	)
	, _maxWordNum(20)//ͳһ�Ĵʱ����ĸ�������ʼ��Ϊ20
{
	std::string stopFileDir = DICT_PATH + "/stop_words.utf8";
	getStopWordSet(stopFileDir.c_str());
	getIDF((DICT_PATH + "/idf.utf8").c_str());
}
//��ȡͣ�ôʣ�����ڶ���_stopWordSet��
void TextSimilarity::getStopWordSet(const char* file)
{
	std::ifstream fin(file);
	if (!fin.is_open())
	{
		cout << file << "open failed!" << endl;
		return;
	}
	std::string word;
	while (!fin.eof())
	{
		getline(fin, word);
		_stopWordSet.insert(word);
	}
	fin.close();
}
//��չ����IDF�ķ�ʽ��ȷ���ؼ���,��ȡ��ͬ�ʵ�IDFȨ��ֵ�������_Idf��
void TextSimilarity::getIDF(const char* file)
{
	std::ifstream fin(file);
	if (!fin.is_open())
	{
		std::cout << "file" << file << "open failed!" << std::endl;
		return;
	}
	//���ַ����ļ�,ͨ���ַ�������������ȡ,�������������
	std::pair<std::string, double> input;
	while (!fin.eof())
	{
		fin >> input.first >> input.second;
		_Idf.insert(input);
	}
	std::cout << "idf size:" << _Idf.size() << endl;
}
//��һ���ı��Ƚ��зִʣ�ȥͣ�ôʣ�֮��ͳ��ÿ���ʵĴ�Ƶ,ͳ�ƺ�Ľ�������wordFreq������
void TextSimilarity::getWordFrep(const char* file, WordFrep& wordFreq)
{
	std::ifstream fin(file);
	if (!fin.is_open())
	{
		cout << file << "open failed!" << endl;
		return;
	}
	std::string line;//�յ�\0
	std::vector<std::string> words;
	while (!fin.eof())//�ļ�û�е�������һֱ��ȡ
	{
		words.clear();
		getline(fin, line);
		//gbk-->utf8
		line = GBK2UTF8(line);
		//�ִ�
		_jieba.Cut(line, words, true);
		//ͳ�ƴ�Ƶ
		for (const auto& wd : words)
		{
			//ȥͣ�ô�
			//if (_stopWordSet.find(wd)!=_stopWordSet.end())
			if (_stopWordSet.count(wd))//��������ֵ1���ڣ�0������,��utf8��ʽ����
				continue;
			//��Ƶ�ۼ�
			//operator[]:insert(make_pair(K,V()),k�����ڣ�����ɹ�������V&  
			wordFreq[wd]++;
		}
	}
	fin.close();
}
//���ı���GBKת����UTF8��ʽ
std::string TextSimilarity::GBK2UTF8(const std::string& gbk)
{
	//gbk--->utf16
	int utf16Bytes = MultiByteToWideChar(CP_ACP, 0, gbk.c_str(), -1, nullptr, 0);//��ȡbuffer��С
	wchar_t* utf16Buffer = new wchar_t[utf16Bytes];
	MultiByteToWideChar(CP_ACP, 0, gbk.c_str(), -1, utf16Buffer, utf16Bytes);//����ת��

	//utf16--->utf8
	int utf8Bytes = WideCharToMultiByte(CP_UTF8, 0, utf16Buffer, -1, nullptr, 0, nullptr, nullptr);
	char* utf8Buffer = new char[utf8Bytes];
	WideCharToMultiByte(CP_UTF8, 0, utf16Buffer, -1, utf8Buffer, utf8Bytes, nullptr, nullptr);

	std::string outString(utf8Buffer);//�����¶���,��utf8Buffer���ݴ����ȥ
	if (utf8Buffer)
		delete[] utf8Buffer;
	if (utf16Buffer)
		delete[] utf16Buffer;
	return outString;
}
//��Դ�Ƶ�����ʴ�Ƶ/���µ�������
void TextSimilarity::getNormalizedWordFrep(WordFrep& wordFreq)
{
	int sz = wordFreq.size();
	for (auto& wf : wordFreq)
	{
		wf.second /= sz;
	}
}
//����TF-IDF��ʽ����Ĵ�Ƶ���洢��outTfIdf
void TextSimilarity::getTfIdf(WordFrep& normalWordFreq, WordFrep& outTfIdf)
{
	for (const auto& wf : normalWordFreq)
	{
		//TFIDF=wf*idf����Դ�Ƶ*idfȨ�ء�������ΪTF-IDF�Ĵ�Ƶ
		outTfIdf.insert(make_pair(wf.first, wf.second * _Idf[wf.first]));
	}
}
//���ݴ�Ƶ���ж��ı��Ĵ��ﰴ��Ƶ���������������Ľ�������outSortedWordFreq��
void TextSimilarity::sortReverseByValue(const WordFrep& wordFreq,
	std::vector<PSI>& outSortedWordFreq)
{
	for (const auto& wf : wordFreq)
	{
		outSortedWordFreq.push_back(wf);
	}
	sort(outSortedWordFreq.begin(), outSortedWordFreq.end(), CMP());
}
//�º������أ����أ��������
struct CMP
{
	//�Ƚϵ���pair�����secondֵ,����������
	bool operator()(const TextSimilarity::PSI& left
		, const TextSimilarity::PSI& right)
	{
		return left.second > right.second;
	}
};
//���Ѿ��Ź���Ĵ�Ƶ��ȡǰ_maxWordNUM���ʣ�����֮�󹹽�ͳһ�Ĵ�����
void TextSimilarity::getWordCode(std::vector<PSI>& inSortWordFreq, WordSet& wordCode)
{
	int len = inSortWordFreq.size();
	int sz = len < _maxWordNum ? len : _maxWordNum;
	for (int i = 0; i < sz; ++i)
	{
		wordCode.insert(inSortWordFreq[i].first);
	}
}
//����ȡ����ǰ_maxWordFreq��len���ʱ�������Ƶ�����������outVec
void TextSimilarity::getVector(WordSet& wordCode, WordFrep& wordFreq, std::vector<double>& outVec)
{
	outVec.clear();
	int sz = wordCode.size();
	outVec.reserve(sz);//��vector��������
	for (const auto& wc : wordCode)
	{
		if (wordFreq.count(wc))
			outVec.push_back(wordFreq[wc]);//�õ����ʶ�Ӧ��Ƶ�ʣ��ŵ�vector��
		else
			outVec.push_back(0);
	}
}
//���ݴ�Ƶ������ʹ���������ƶȼ��������ı������ƶ�
double TextSimilarity::getCosine(std::vector<double>& vec1, std::vector<double>& vec2)
{
	if (vec1.size() != vec2.size())
		return 0;
	double product = 0.0;
	double modual1 = 0.0, modual2 = 0.0;
	//����
	for (int i = 0; i < vec1.size(); ++i)
	{
		product += vec1[i] * vec2[i];
	}
	//��ĸ
	for (int i = 0; i < vec1.size(); ++i)
	{
		modual1 += pow(vec1[i], 2);
	}
	modual1 = pow(modual1, 0.5);
	for (int i = 0; i < vec2.size(); ++i)
	{
		modual2 += pow(vec2[i], 2);
	}
	modual2 = pow(modual2, 0.5);
	return product / (modual2 * modual1);
}
void testWordReverse()
{
	TextSimilarity ts("dict");
	TextSimilarity::WordFrep wf;
	TextSimilarity::WordFrep tfidf;
	ts.getWordFrep("test.txt", wf);
	ts.getNormalizedWordFrep(wf);
	//ts.printStopWordSet();
	//ts.printWordFrep(wf);
	ts.getTfIdf(wf, tfidf);

	std::vector<TextSimilarity::PSI> vecFreq;
	std::vector<TextSimilarity::PSI> vecTfidf;
	ts.sortReverseByValue(wf, vecFreq);
	ts.sortReverseByValue(tfidf, vecTfidf);
	std::cout << "sorted word freq:" << std::endl;
	for (const auto& wf : vecFreq)
	{
		std::cout << ts.UTF82GBK(wf.first) << ": " << wf.second << " ";
	}
	std::cout << std::endl;

	std::cout << "sorted tfidf:" << std::endl;
	for (const auto& wf : vecTfidf)
	{
		std::cout << ts.UTF82GBK(wf.first) << ": " << wf.second << " ";
	}
	std::cout << std::endl;

	TextSimilarity::WordFrep wf2;
	TextSimilarity::WordFrep tfidf2;
	ts.getWordFrep("test2.txt", wf2);
	ts.getNormalizedWordFrep(wf2);
	ts.getTfIdf(wf2, tfidf2);
	std::vector<TextSimilarity::PSI> vecFreq2;
	std::vector<TextSimilarity::PSI> vecTfidf2;
	ts.sortReverseByValue(wf2, vecFreq2);
	ts.sortReverseByValue(tfidf2, vecTfidf2);
	std::cout << "sorted word freq2:" << std::endl;
	for (const auto& wf : vecFreq2)
	{
		std::cout << ts.UTF82GBK(wf.first) << ": " << wf.second << " ";
	}
	std::cout << std::endl;

	std::cout << "sorted word tfidf2:" << std::endl;
	for (const auto& wf : vecTfidf2)
	{
		std::cout << ts.UTF82GBK(wf.first) << ": " << wf.second << " ";
	}
	std::cout << std::endl;

	//set�Ǳ����洢����ȥ�ص�
	TextSimilarity::WordSet wordCode;
	TextSimilarity::WordSet wordCode2;
	TextSimilarity::WordSet wordTFIDFCode;

	//����ͳһ�Ĵʱ�wordCode��wordCode�Ĵ�С<=40
	ts.getWordCode(vecFreq, wordCode);
	ts.getWordCode(vecFreq2, wordCode);

	//����ʹ��TFIDF��������Ĵ�Ƶ��ͳһ�ʱ������wordCode2
	ts.getWordCode(vecTfidf, wordCode2);
	ts.getWordCode(vecTfidf2, wordCode2);
	std::cout << "wf worfcode:" << std::endl;
	for (const auto& wc : wordCode)
	{
		std::cout << ts.UTF82GBK(wc) << " ";
	}
	std::cout << std::endl;

	std::cout << "TFidf worfcode:" << std::endl;
	for (const auto& wc : wordCode2)
	{
		std::cout << ts.UTF82GBK(wc) << " ";
	}
	std::cout << std::endl;

	std::vector<double> Vec1;
	std::vector<double> Vec2;

	std::vector<double> VecTfidf1;
	std::vector<double> VecTfidf2;

	//�����ı�1���ı�2�Ĵ�Ƶ�������ֱ�����Vec1��Vec2
	ts.getVector(wordCode, wf, Vec1);
	ts.getVector(wordCode, wf2, Vec2);

	ts.getVector(wordCode2, wf, VecTfidf1);
	ts.getVector(wordCode2, wf2, VecTfidf2);
	std::cout << "vec1:" << std::endl;
	for (const auto& e : Vec1)
	{
		std::cout << e << " ";
	}
	std::cout << std::endl;
	std::cout << "vec2:" << std::endl;
	for (const auto& e : Vec2)
	{
		std::cout << e << " ";
	}
	std::cout << std::endl;

	std::cout << "vecTfidf1:" << std::endl;
	for (const auto& e : VecTfidf1)
	{
		std::cout << e << " ";
	}
	std::cout << std::endl;
	std::cout << "vecTfidf2" << std::endl;
	for (const auto& e : VecTfidf2)
	{
		std::cout << e << " ";
	}
	std::cout << std::endl;

	//�����ı����ƶ�
	std::cout << ts.getCosine(Vec1, Vec2) << std::endl;
	std::cout << ts.getCosine(VecTfidf1, VecTfidf2) << std::endl;
}