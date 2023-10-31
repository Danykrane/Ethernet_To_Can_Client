#ifndef __TUPLE_ADDRESS_PARSER_HPP__
#define __TUPLE_ADDRESS_PARSER_HPP__

#include <iostream>
#include <list>
#include <tuple>
#include <vector>

// FIXME: Не работает подсветка и требуется пересобирать весь проект
// #if __cplusplus <= 201103
// #define __TAP_USE_QT_REGEXP__
// #endif

// Если закомментированно, то используется std::regex, иначе QRegExp
// #define __TAP_USE_QT_REGEXP__

#ifdef __TAP_USE_QT_REGEXP__
#include <QRegExp>
#else
#include <regex>
#endif

//! Tuple address parser namespace
namespace tap
{
//! Последовательность из индексов от N до 1
template<size_t... Idxs>
struct __idx_sequence
{
};

//! Генератор последовательности индексов
template<size_t idx, size_t... Idxs>
struct __generate_sequence : __generate_sequence<idx - 1, idx, Idxs...>
{
};

//! Специализация генератора для idx=1
template<size_t... Idxs>
struct __generate_sequence<1, Idxs...> : __idx_sequence<1, Idxs...>
{
};
//! Возвращает регулярку для указанного типа
template<class A>
std::string regexForType(A);

template<>
std::string regexForType(int);
template<>
std::string regexForType(unsigned int);
template<>
std::string regexForType(double);
template<>
std::string regexForType(std::string);

//! Преобразовывает строку в указанный тип
template<class A>
bool fromStringForType(std::string str, A& arg);
template<>
bool fromStringForType<int&>(std::string str, int& arg);
template<>
bool fromStringForType<double&>(std::string str, double& arg);
template<>
bool fromStringForType<std::string&>(std::string str, std::string& arg);

//! Парсит строку в элемент кортежа
template<size_t idx, class... Ts>
bool __parseTupleElement (std::vector<std::string>& match, std::tuple<Ts...>& t)
{
    constexpr size_t id = idx - 1;
    return fromStringForType<decltype(std::get<id>(t))>(match[id],
                                                        std::get<id>(t));
};

//! Парсит элементы кортежа для указанных индексов
template<class... Ts, size_t... Idxs>
bool __parseTupleForIndexSequence (std::vector<std::string>& match,
                                   std::tuple<Ts...>&        t,
                                   __idx_sequence<Idxs...>)
{
    std::initializer_list<bool> results
        = {__parseTupleElement<Idxs>(match, t)...};

    return std::all_of(results.begin(), results.end(), [] (const bool& b) {
        return b;
    });
}

//! Добавляет в список строку для регулярки
template<size_t idx, class... Ts>
void __fillRegexListForElement (std::list<std::string>& regexes,
                                std::tuple<Ts...>&      t)
{
    constexpr size_t id = idx - 1;
    regexes.push_back(regexForType(std::get<id>(t)));
};

//! Добавляет в список строки для регулярок
template<class... Ts, size_t... Idxs>
void __fillRegexList (std::list<std::string>& regexes,
                      std::tuple<Ts...>&      t,
                      __idx_sequence<Idxs...>)
{
    std::initializer_list<int> dummy_list{
        (__fillRegexListForElement<Idxs>(regexes, t), 0)...};

    (void)(dummy_list);
}

//! Верифициирует строку и разбивает ее на токены
bool __splitForTokens(const std::string&            str,
                      const std::list<std::string>& tokensRegexes,
                      std::vector<std::string>&     tokens,
                      char                          delimiter);

//! Парсинг строки в кортеж
template<class... Ts>
bool parseTuple (std::tuple<Ts...>& t, std::string str, char delimiter = '/')
{
    // Длина кортежа
    constexpr size_t tupleSize = sizeof...(Ts);
    // Последовательность индексов для get<>
    constexpr auto sequence = __generate_sequence<tupleSize>();

    // Создание регулярок для каждого типа
    std::list<std::string> regexes;
    __fillRegexList(regexes, t, sequence);

    if (regexes.size() != tupleSize)
    {
        return false;
    }

    // Оборачивает регулярки в пробелы
    for (auto& rg: regexes)
    {
        rg = "\\s*(" + rg + ")\\s*";
    }

    // Разбиение на токены
    std::vector<std::string> tokens;
    if (!__splitForTokens(str, regexes, tokens, delimiter))
    {
        return false;
    }

    // Преобразование токенов в кортеж
    return __parseTupleForIndexSequence(tokens, t, sequence);
}

//! Парсинг строки в кортеж.
//! Последний элемент кортежа true, если операция завершилась успешно
template<class... Ts>
std::tuple<Ts..., bool> parse (const std::string& str, char delimiter = '/')
{
    std::tuple<Ts...>       t0;
    bool                    res = parseTuple(t0, str, delimiter);
    std::tuple<Ts..., bool> t1  = std::tuple_cat(t0, std::tie(res));
    return t1;
}

//! Парсинг строки в кортеж.
//! Последний элемент кортежа true, если операция завершилась успешно
template<class... Ts>
bool parse (Ts&... args, const std::string& str, char delimiter = '/')
{
    bool                    res;
    std::tuple<Ts..., bool> t;
    t                      = parse<Ts...>(str, delimiter);
    std::tie(args..., res) = t;
    return res;
}

} // namespace tap

template<class... Ts>
class FormatParser
{
public:
    explicit FormatParser(const std::string& str, char delimiter = '/')
        : m_str(str)
        , m_delimeter(delimiter)
        , m_parseResult(false)
    {
    }

    FormatParser() = delete;

    template<class... Refs>
    bool apply (Refs&... args)
    {
        m_parseResult = false;
        std::tie(args..., m_parseResult)
            = tap::parse<Ts...>(m_str, m_delimeter);
        return m_parseResult;
    }

    bool success () const
    {
        return m_parseResult;
    }
    bool hasError () const
    {
        return !success();
    }

private:
    const std::string m_str;
    const char        m_delimeter;
    bool              m_parseResult;
};

#endif //__TUPLE_ADDRESS_PARSER_HPP__
