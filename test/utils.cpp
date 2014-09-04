#include "globals.h"
#include <mlang.h>

LPWSTR load_text_file( LPCWSTR path, bool is_html, LPCWSTR defEncoding )
{
	CoInitialize(NULL);

	LPWSTR strW = NULL;

	HANDLE fl = CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(fl != INVALID_HANDLE_VALUE)
	{
		DWORD size = GetFileSize(fl, NULL);
		LPSTR str = (LPSTR) malloc(size + 1);

		DWORD cbRead = 0;
		ReadFile(fl, str, size, &cbRead, NULL);
		str[cbRead] = 0;
		CloseHandle(fl);

		int bom = 0;
		if(str[0] == '\xEF' && str[1] == '\xBB' && str[2] == '\xBF')
		{
			bom = 3;
		}

		if(is_html)
		{
			std::wstring encoding;
			char* begin = StrStrIA(str, "<meta");
			while(begin && encoding.empty())
			{
				char* end = StrStrIA(begin, ">");
				char* s1 = StrStrIA(begin, "Content-Type");
				if(s1 && s1 < end)
				{
					s1 = StrStrIA(begin, "charset");
					if(s1)
					{
						s1 += strlen("charset");
						while(!isalnum(s1[0]) && s1 < end)
						{
							s1++;
						}
						while((isalnum(s1[0]) || s1[0] == '-') && s1 < end)
						{
							encoding += s1[0];
							s1++;
						}
					}
				}
				if(encoding.empty())
				{
					begin = StrStrIA(begin + strlen("<meta"), "<meta");
				}
			}

			if(encoding.empty() && defEncoding)
			{
				encoding = defEncoding;
			}

			if(!encoding.empty())
			{
				IMultiLanguage* ml = NULL;
				HRESULT hr = CoCreateInstance(CLSID_CMultiLanguage, NULL, CLSCTX_INPROC_SERVER, IID_IMultiLanguage, (LPVOID*) &ml);	

				MIMECSETINFO charset_src = {0};
				MIMECSETINFO charset_dst = {0};

				BSTR bstrCharSet = SysAllocString(encoding.c_str());
				ml->GetCharsetInfo(bstrCharSet, &charset_src);
				SysFreeString(bstrCharSet);

				bstrCharSet = SysAllocString(L"utf-8");
				ml->GetCharsetInfo(bstrCharSet, &charset_dst);
				SysFreeString(bstrCharSet);

				DWORD dwMode = 0;
				UINT  szDst = (UINT) strlen(str) * 4;
				LPSTR dst = new char[szDst];

				if(ml->ConvertString(&dwMode, charset_src.uiInternetEncoding, charset_dst.uiInternetEncoding, (LPBYTE) str + bom, NULL, (LPBYTE) dst, &szDst) == S_OK)
				{
					dst[szDst] = 0;
					cbRead = szDst;
					delete str;
					str = dst;
					bom = 0;
				} else
				{
					delete dst;
				}
			}
		}

		if(!strW)
		{
			strW = new WCHAR[cbRead + 1];
			MultiByteToWideChar(CP_UTF8, 0, str + bom, -1, strW, cbRead + 1);
		}

		free(str);
	}

	CoUninitialize();

	return strW;
}
