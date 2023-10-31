#include "tupleaddressparser.h"

namespace tap
{

// Регулярные выраженгия для типов
template<>
std::string regexForType (int)
{
    return "-?\\d+";
}

template<>
std::string regexForType (unsigned int)
{
    return "\\d+";
}

template<>
std::string regexForType (double)
{
    return "[-]?\\d+[.]\\d+(\\s*[a-zA-Z0-9_.-]*)*";
}
template<>
std::string regexForType (std::string)
{
    return "[a-zA-Z0-9_.-]+";
}

// Преобразование строки в тип
template<>
bool fromStringForType<int&>(std::string str, int& arg)
{
    arg = atoi(str.c_str());
    return true;
}

template<>
bool fromStringForType<unsigned int&>(std::string str, unsigned int& arg)
{
    arg = atoi(str.c_str());
    return true;
}

template<>
bool fromStringForType<double&>(std::string str, double& arg)
{
    arg = atof(str.c_str());
    return true;
}

template<>
bool fromStringForType<std::string&>(std::string str, std::string& arg)
{
    arg = str;
    return true;
}

//
bool __splitForTokens (const std::string&            str,
                       const std::list<std::string>& tokensRegexes,
                       std::vector<std::string>&     tokens,
                       char                          delimiter)
{
    std::list<std::string> regexes = tokensRegexes;

    // Создание регулярки для всего выражения
    std::string regexStr;
    for (std::list<std::string>::iterator i = regexes.begin();
         i != regexes.end();
         i++)
    {
        if (i != regexes.begin())
        {
            regexStr += delimiter;
        }
        regexStr += (*i);
    }

    regexStr += ".*";

    // Проверка всего выражения
#ifdef __TAP_USE_QT_REGEXP__ // для QRegExp
    QRegExp rg(QString::fromStdString(regexStr));

    if (rg.indexIn(QString::fromStdString(str)) < 0)
    {
        return false;
    }

#else // для std::regex
    std::regex  rg(regexStr);
    std::smatch sm;

    if (!std::regex_match(str, sm, rg))
    {
        return false;
    }
#endif

    // Поиск токенов для каждой регулярки
    tokens.clear();

#ifdef __TAP_USE_QT_REGEXP__ // для QRegExp
    QString tempStr = QString::fromStdString(str);
    int     lastIdx = 0;
    while (!regexes.empty())
    {
        rg = QRegExp(QString::fromStdString(regexes.front()));
        regexes.pop_front();

        lastIdx = rg.indexIn(tempStr, lastIdx);

        // Выход, если регулярка не найдена
        if (lastIdx >= 0)
        {
            tokens.push_back(rg.cap(1).toStdString());
            lastIdx += rg.cap(0).length();
        }
        else
        {
            return false;
        }
    }
#else // для std::regex
    std::string tempStr = str;
    while (!regexes.empty())
    {
        rg = regexes.front();
        regexes.pop_front();

        // Выход, если регулярка не найдена
        if (std::regex_search(tempStr, sm, rg))
        {
            tokens.push_back(sm.str(1));
            tempStr = sm.suffix();
        }
        else
        {
            return false;
        }
    }
#endif

    return tokens.size() == tokensRegexes.size();
}

} // namespace tap
