#include "md5.cpp"
#include <functional>
#include <chrono>


//Класс для обработки и вывода исключений пользователю
class Exception : public exception
{
protected:
    //сообщение об ошибке
    char* str;
public:
    Exception(const char* s)
    {
        str = new char[strlen(s) + 1];
        strcpy_s(str, strlen(s) + 1, s);
    }
    Exception(const Exception& e)
    {
        str = new char[strlen(e.str) + 1];
        strcpy_s(str, strlen(e.str) + 1, e.str);
    }
    ~Exception()
    {
        delete[] str;
    }
    virtual void print()
    {
        cout << "Exception: " << str << "; " << what();
    }
};
//Вывод исключения пользователю при вводе неправильного типа данных
class UnsuitableDataType : public Exception
{
public:
    UnsuitableDataType(const char* s) : Exception(s) {}
    virtual void print()
    {
        cout << "UnsuitableDataType; " << what();
    }
};
//Вывод исключения пользователю при вводе неправильного размера таблицы
class IncorrectSize : public Exception
{
public:
    IncorrectSize(const char* s) : Exception(s) {}
    virtual void print()
    {
        cout << "IncorrectSize; " << what();
    }
};

// Шаблонный класс для хранения узла связного списка
template <typename T, typename V>
class Node {
public:
    // Ключ и значение
    T key;
    V value;
    // Указатель на следующий узел
    Node* next;
    Node(T key, V value) {
        this->key = key;
        this->value = value;
        this->next = nullptr;
    }
};

// Шаблонный класс для реализации хеш-таблицы
template <typename T, typename V>
class HashTable {
private:
    // Размер хеш-таблицы
    int size;
    // Кол-во элементов
    int count;
    // Массив указателей на связные списки
    Node<T, V>** table;
    // Логическая переменная для определения того, будет ли память для хеш-таблицы автоматически 
    // перевыделяться в случае превышения допустимых границ коэфф-а заполненности
    bool autoResize = false;
    // Функция для вычисления хеш-кода ключа с помощью алгоритма MD5
    int hash(const T& key) {
        //Проверка, верный ли тип данных хешируется
        if (typeid(key) == typeid(string) || typeid(key) == typeid(int)) {
            //Преобразование Т в string, тк MD5 работает со строками
            string key_str;
            if constexpr (is_same_v<T, int>) {
                key_str = to_string(key);
            }
            else key_str = key;
            //Вычисление хеша
            string hash = hexString(md5(key_str));
            //Вычисление индекса
            int index = 0;
            // суммирование всех байт хеша
            for (int i = 0; i < hash.length(); i++) {
                index += hash[i];
            }
            return index%size;
        }
        else {
            throw UnsuitableDataType("\nAttempt to hash unsuitable data type");
        }
    }
public:
    HashTable(int size = 5) {
        if(size<=0) throw IncorrectSize("\nAttempt to create hash-table with negative size");
        this->size = size;
        this->count = 0;
        // Выделяем память для массива указателей
        table = new Node<T, V>*[size];
        // Инициализируем все указатели нулевыми значениями
        for (int i = 0; i < size; i++) {
            table[i] = nullptr;
        }
    }
    //Конструктор копий
    HashTable(const HashTable& other) {
        // Копируем размер массива
        this->size = other.size;
        this->count = other.count;
        // Выделяем память под массив связных списков
        table = new Node<T, V>* [size];
        // Проходим по каждому списку в массиве
        for (int i = 0; i < size; i++) {
            // Копируем голову списка
            Node<T, V>* head = other.table[i];
            if (head == nullptr) {
                table[i] = nullptr;
            }
            else {
                // Если список не пуст, то создаем новый узел с тем же ключом и значением
                table[i] = new Node<T, V>(head->key, head->value);
                Node<T, V>* current = table[i]; // Текущий узел в новом списке
                Node<T, V>* next = head->next; // Следующий узел в старом списке
                while (next != nullptr) {
                    // Создаем новый узел с тем же ключом и значением
                    current->next = new Node<T, V>(next->key, next->value);
                    // Переходим к следующим узлам в обоих списках
                    current = current->next;
                    next = next->next;
                }
            }
        }
    }

    // Деструктор
    ~HashTable() {
        // Освобождаем память для каждого связного списка
        for (int i = 0; i < size; i++) {
            Node<T, V>* current = table[i];
            while (current != nullptr) {
                Node<T, V>* next = current->next;
                delete current;
                current = next;
            }
        }
        // Освобождаем память для массива указателей
        delete[] table;
    }
    int getCount() { //кол-во элементов
        return count;
    }
    int getSize() { //размер
        return size;
    }
    double getCoef() { //коэф. заполненности 
        return count*1.0 /size*1.0;
    }
    // сеттер параметра автоматического перевыделения памяти
    void setAutoResize(bool temp) {
        autoResize = temp;
    }

    //Функция для перевыделения памяти в случае, если таблица потребляет слишком много памяти(коэф. заполненности<0.25)
    //либо слишком сильно заполнена(коэф. заполненности>0.5)
    void resize(int newSize) {
        if(newSize<=0) throw IncorrectSize("\nAttempt to resize hash-table to negative or zero size");
        Node<T,V>** newTable = new Node<T,V>* [newSize]; // создаем новый массив
        for (int i = 0; i < newSize; i++) { //заполняем нулевыми указателями
            newTable[i] = nullptr;
        }
        int oldSize = size;
        size = newSize;
        for (int i = 0; i < oldSize; i++) {  
            Node<T,V>* current = table[i];
            while (current != nullptr) {
                int index = hash(current->key);  //Повторное хеширование ключа и вычисление ячейки
                Node<T,V>* temp = current->next;
                current->next = newTable[index];  
                newTable[index] = current;
                current = temp;
            }
        }
        delete[] table;  //освобождение старой памяти
        table = newTable;
    }

    // Метод для вставки элемента в хеш-таблицу
    void insert(T key, V value) {
        // Вычисляем хеш-код ключа
        int index = hash(key);
        // Создаем новый узел с ключом и значением
        Node<T, V>* node = new Node<T, V>(key, value);
        // Если указатель в ячейке не указывает на элемент, то делаем новый узел головой списка
        if (table[index] == nullptr) {
            table[index] = node;
            count++;
        }
        else {
            // Иначе ищем место для вставки узла в конец или середину списка
            Node<T, V>* current = table[index];
            while (current->next != nullptr && current->key != key) {
                current = current->next;
            }
            // Если нашли узел с таким же ключом, то обновляем его значение
            if (current->key == key) {
                current->value = value;
                delete node; // Удаляем созданный ранее узел, тк просто обновили значение у уже существующего
                node = nullptr;
            }
            else {
                // Иначе вставляем новый узел в конец списка
                current->next = node;
                count++;
            }
        }
        if (count * 1.0 / size * 1.0 >= 0.75 && autoResize == true) resize(size * 2);
    }

    // Метод для поиска элемента в хеш-таблице по ключу
    V search(T key) {
        // Вычисляем хеш-код ключа
        int index = hash(key);
        // Ищем узел с таким же ключом в связном списке по индексу таблицы
        Node<T, V>* current = table[index];
        while (current != nullptr && current->key != key) {
            current = current->next;
        }
        // Если нашли такой узел, то возвращаем его значение
        if (current != nullptr) {
            return current->value;
        }
        else {
            // Иначе возвращаем -1 или None, что означает отсутствие элемента с таким ключом
            if constexpr (is_same_v<V, int>) {
                return -1;
            }
            else{
                return "None";
            }
        }
    }

    // Метод для удаления элемента из хеш-таблицы по ключу
    void remove(T key) {
        // Вычисляем хеш-код ключа
        int index = hash(key);
        // Ищем узел с таким же ключом в связном списке по индексу таблицы
        Node<T, V>* current = table[index];
        Node<T, V>* previous = nullptr;
        while (current != nullptr && current->key != key) {
            previous = current;
            current = current->next;
        }
        // Если нашли такой узел, то удаляем его из списка
        if (current != nullptr) {
            // Если узел был головой списка, то обновляем указатель в таблице
            if (previous == nullptr) {
                table[index] = current->next;
            }
            else {
                // Иначе обновляем указатель предыдущего узла
                previous->next = current->next;
            }
            // Освобождаем память для удаленного узла
            delete current;
            current = nullptr;
            count--;
        }
        if (count * 1.0 / size * 1.0 <= 0.2 && size > 10 && autoResize == true) { // если коэффициент заполнения меньше 0.25 и размер больше 10
            resize(size / 2); // уменьшаем размер массива в два раза
        }
    }

    // Метод для вывода содержимого хеш-таблицы на экран
    void print() {
        cout << "\n";
        // Проходим по всем ячейкам таблицы
        for (int i = 0; i < size; i++) {
            // Выводим индекс ячейки
            cout << i << ": ";
            // Выводим все элементы связного списка по этому индексу
            Node<T, V>* current = table[i];
            while (current != nullptr) {
                cout << "(" << current->key << ", " << current->value << ") -> ";
                current = current->next;
            }
            cout << "NULL" << "\n";
        }
    }
    // Удобный для пользователя интефрейс ввода
    void input(HashTable<T,V>& table, int count_of_keys = 2) {
        T user_key; V user_value;
        string input_key; string input_value; // строки для считывания ввода
        for (int i = 0; i < count_of_keys; i++) {
            cout << "Key: " << "\n";
            cin >> input_key; // считываем строку
            if (typeid(T) == typeid(int)) { // если T - это int
                try {
                    user_key = stoi(input_key); // пытаемся преобразовать в int
                }
                catch (const invalid_argument& e) { //Если не получается, значит пользователь ввел неверный тип данных
                    throw UnsuitableDataType("\nAttempt to input unsuitable data type key as int");
                }
            }
            else if constexpr (is_same_v<T, string>) { // если T - это string
                user_key = input_key; // просто присваиваем строку
            }
            else {
                // T - это неизвестный тип данных
                throw UnsuitableDataType("\nUnknown data type for key");
            }
            cout << "Value: " << "\n";
            cin >> input_value; // считываем строку
            if (typeid(V) == typeid(int)) { // если V - это int
                try {
                    user_value = stoi(input_value); // пытаемся преобразовать в int
                }
                catch (const invalid_argument& e) { //Если не получается, значит пользователь ввел неверный тип данных
                    throw UnsuitableDataType("\nAttempt to input unsuitable data type value as int");
                }
            }
            else if constexpr (is_same_v<V, string>) { // если V - это string
                user_value = input_value; // просто присваиваем строку
            }
            else {
                // V - это неизвестный тип данных
                throw UnsuitableDataType("\nUnknown data type for value");
            }
            table.insert(user_key, user_value);
            count++;
        }
    }
    //Оператор=
    HashTable<T, V>& operator=(const HashTable<T, V>& other) {
        // Проверяем на самоприсваивание
        if (this == &other) {
            return *this;
        }
        // Освобождаем память от старого массива связных списков
        for (int i = 0; i < size; i++) {
            Node<T,V>* current = table[i];
            while (current != nullptr) {
                Node<T, V>* next = current->next;
                delete current;
                current = next;
            }
        }
        delete[] table;
        // Копируем размер массива
        size = other.size;
        count = other.count;
        // Выделяем память под новый массив связных списков
        table = new Node<T, V>* [size];
        // Проходим по каждому списку в массиве
        for (int i = 0; i < size; i++) {
            // Копируем голову списка
            Node<T, V>* head = other.table[i];
            if (head == nullptr) {
                // Если список пуст, то присваиваем nullptr
                table[i] = nullptr;
            }
            else {
                // Если список не пуст, то создаем новый узел с тем же ключом и значением
                table[i] = new Node<T, V>(head->key, head->value);
                Node<T, V>* current = table[i]; // Текущий узел в новом списке
                Node<T, V>* next = head->next; // Следующий узел в старом списке
                while (next != nullptr) {
                    // Создаем новый узел с тем же ключом и значением
                    current->next = new Node<T,V>(next->key, next->value);
                    // Переходим к следующим узлам в обоих списках
                    current = current->next;
                    next = next->next;
                }
            }
        }
        // Возвращаем текущий объект
        return *this;
    }
};

int main() {
    // Создаем хеш-таблицу с размером 5 для строковых ключей и целочисленных значений
    HashTable<string, int> ht1(5);
    // Вставляем несколько элементов в хеш-таблицу
    //Имя - Возраст
    ht1.insert("Alex", 22);
    ht1.insert("Ivan", 25);
    ht1.insert("Grigoriy", 16);
    ht1.insert("Katya", 18);
    ht1.insert("Evgeniy", 20);
    // Выводим содержимое хеш-таблицы на экран
    cout << "Hash table with string keys:" << "\n";
    ht1.print();
    cout << "\n";
    // Поиск элементов в хеш-таблице по ключу
    cout << "Search for key Alex: " << ht1.search("Alex") << "\n";
    cout << "Search for key Katya: " << ht1.search("Katya") << "\n";
    cout << "Search for key Evgeniy: " << ht1.search("Evgeniy") << "\n";
    cout << "\n";
    // Удаление элементов из хеш-таблицы по ключу
    cout << "Remove key Alex" << "\n";
    ht1.remove("Alex");
    cout << "Remove key Katya" << "\n";
    ht1.remove("Katya");
    cout << "Remove key Evgeniy" << "\n";
    ht1.remove("Evgeniy");
    cout << "\n";
    // Выводим содержимое хеш-таблицы на экран после удаления
    cout << "Hash table after removal:" << "\n";
    ht1.print();
    cout << "\n";

    //Работа функции ввода через консоль
    cout << "\nInt keys; String values; Size: 10;";
    cout << "\nEnter 5 elements: \n";
    HashTable<int, string> ht5(10);
    try
    {
        ht5.input(ht5,5);
    }
    catch (Exception e)
    {
    	cout << "\nException has been caught: "; e.print();
    }
    ht5.print();

    

    //объект класса для копирования
    //HashTable<int, string> ht_to_copy(5);
    //ht_to_copy.insert(101, "010");

    //Работа конструктора копий
    //HashTable<int, string> ht6(ht_to_copy);
    //cout << "ht6: " << "\n";
    //ht6.print();

    //Работа оператора=
    //HashTable<int, string> ht7(5);
    //ht7 = ht_to_copy;
    //cout << "ht7: " << "\n";
    //ht7.print();

    //работа get-теров
 /* HashTable<int, int> ht8(100);
    for (int i = 0; i < 40; i++) {
        ht8.insert(i, i * i);
    }
    ht8.print();
    cout << "Count of items: " << ht8.getCount() << "\n";
    cout << "Size of hashtable: " << ht8.getSize() << "\n";
    cout << "Occupancy rate: " << ht8.getCoef() << "\n";*/

    //Измерение времени вставки 10000 элементов в хеш-таблицу <int,int> (микросекунды)
    //HashTable<int, int> ht10(100);
    //int time = 0;
    //for (int i = 0; i < 10000; i++) {
    //    int temp = i * 15; //как значение возьмем ключ, умноженный на константу
    //    chrono::steady_clock::time_point begin = chrono::steady_clock::now(); //начало отсчета
    //    ht10.insert(i, temp); //вставка
    //    chrono::steady_clock::time_point end = chrono::steady_clock::now(); //конец отсчета
    //    time += chrono::duration_cast<std::chrono::microseconds> (end - begin).count(); //добавление времени данной итерации к итоговой сумме
    //   //cout << "Round " << i << ": " << time << "\n";
    //}
    //cout << "Insert. Sum: " << time << "\n";
    //Измерение времени поиска 
    //int time1 = 0;
    //for (int i = 0; i < 10000; i++) {
    //    chrono::steady_clock::time_point begin = chrono::steady_clock::now();
    //    ht10.search(i);
    //    chrono::steady_clock::time_point end = chrono::steady_clock::now();
    //    time1 += chrono::duration_cast<std::chrono::microseconds> (end - begin).count();
    //    //cout << "Round " << i << ": " << time1 << "\n";
    //}
    //cout << "Search. Sum: " << time1 << "\n";
    //Измерение времени удаления 
    //int time2 = 0;
    //for (int i = 0; i < 10000; i++) {
    //    chrono::steady_clock::time_point begin = chrono::steady_clock::now();
    //    ht10.remove(i);
    //    chrono::steady_clock::time_point end = chrono::steady_clock::now();
    //    time2 += chrono::duration_cast<std::chrono::microseconds> (end - begin).count();
    //    //cout << "Round " << i << ": " << time2 << "\n";
    //}
    //cout << "Remove. Sum: " << time2 << "\n";

    int c;  cin >> c;
    return 0;
}
