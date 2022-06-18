#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <concepts>
using namespace std;

struct list_node {
    int data;
    list_node *next;
};

template<bool Const>
class list_of_ints_iterator {
    friend class list_of_ints;
    friend class list_of_ints_iterator<!Const>;
    using node_pointer = std::conditional_t<Const, const list_node*, list_node*>;
    node_pointer ptr_;
    explicit list_of_ints_iterator(node_pointer p) : ptr_(p) {}
public:
    using difference_type = std::ptrdiff_t;
    using value_type = int;
    using pointer = std::conditional_t<Const, const int*, int*>;
    using reference = std::conditional_t<Const, const int&, int&>;
    using iterator_category = std::forward_iterator_tag;
    reference operator*() const { return ptr_->data; }
    auto& operator++() { ptr_ = ptr_->next; return *this; }
    auto operator++(int) { auto result = *this; ++*this; return result; }
    template<bool R>
    bool operator==(const list_of_ints_iterator<R>& rhs) const
    { return ptr_ == rhs.ptr_; }
    template<bool R>
    bool operator!=(const list_of_ints_iterator<R>& rhs) const
    { return ptr_ != rhs.ptr_; }
    operator list_of_ints_iterator<true>() const
    { return list_of_ints_iterator<true>{ptr_}; }
};

class list_of_ints {
    list_node *head_ = nullptr;
    list_node *tail_ = nullptr;
    int size_ = 0;
public:
    using const_iterator = list_of_ints_iterator<true>;
    using iterator = list_of_ints_iterator<false>;
    iterator begin() { return iterator{head_}; }
    iterator end() { return iterator{nullptr}; }
    const_iterator begin() const { return const_iterator{head_}; }
    const_iterator end() const { return const_iterator{nullptr}; }
    int size() const { return size_; }
    void push_back(int value) {
        list_node *new_tail = new list_node{value, nullptr};
        if (tail_) {
            tail_->next = new_tail;
        } else {
            head_ = new_tail;
        }
        tail_ = new_tail;
        size_ += 1;
    }
    ~list_of_ints() {
        for (list_node *next, *p = head_; p != nullptr; p = next) {
            next = p->next;
            delete p;
        }
    }
};

template<typename Iterator>
auto distance(Iterator begin, Iterator end)
{
    using Traits = std::iterator_traits<Iterator>;
    if constexpr (std::is_base_of_v<std::random_access_iterator_tag,
            typename Traits::iterator_category>) {
        return (end - begin);
    } else {
        auto result = typename Traits::difference_type{};
        for (auto it = begin; it != end; ++it) {
            ++result;
        }
        return result;
    }
}
template<typename Iterator, typename Predicate>
auto count_if(Iterator begin, Iterator end, Predicate pred)
{
    using Traits = std::iterator_traits<Iterator>;
    auto sum = typename Traits::difference_type{};
    for (auto it = begin; it != end; ++it) {
        if (pred(*it)) {
            ++sum;
        }
    }
    return sum;
}

class IObserver {
public:
    virtual void update(double p) = 0;
};

class IObservable {
public:
    virtual void addObserver(IObserver* o) = 0;
    virtual void removeObserver(IObserver* o) = 0;

    virtual void notify() = 0;
    virtual ~IObservable() {}
};

class Product : public IObservable {
private:
    vector<IObserver*> observers;
    double price;
    list_of_ints historyOfPrices;
public:
    list_of_ints history = historyOfPrices;
    Product(double p) : price(p) {}
    void changePrice(double p) {
        price = p;
        notify();
        historyOfPrices.push_back(p); // каждый раз обновляя цену добавляем её в нашу историю цены
        for (auto i:historyOfPrices) {  // печатаем нашу цену через range based for
            cout << i << " ";
        }
        cout << endl;
    }
    void addObserver(IObserver* o) override {
        observers.push_back(o);
    }
    void removeObserver(IObserver* o) override {
        observers.erase(remove(observers.begin(), observers.end(), o), observers.end());
    }
    void notify() override {
        for (auto o : observers) {
            o->update(price);
        }
    }
};

class Wholesaler : public IObserver {
private:
    IObservable* product;
public:
    Wholesaler(IObservable* obj) : product(obj) {
        obj->addObserver(this);
    }

    void update(double p) override {
        if (p < 300) {
            cout << "Wholesaler have bought product with price : " << p << endl;
            product->removeObserver(this);
        }
    }
};

class Buyer : public IObserver {
private:
    IObservable* product;
public:
    Buyer(IObservable* obj) : product(obj) {
        obj->addObserver(this);
    }
    void update(double p) override {
        if (p < 350) {
            cout << "Common buyer have bought product with price : " << p << endl;
            product->removeObserver(this);
        }
    }
};


template<class T>
concept IterToComparable =  // назвали наше ограничение
requires(T a, T b) {
    {*a < *b} -> std::convertible_to<bool>; // результат сравнения что-то конвертируемое в bool, то есть сам bool например
};


template<IterToComparable InputIt>
void SortDefaultComparator(InputIt begin, InputIt end) {
    std::sort(begin, end);
}

struct X {
    auto operator<=>(const X&) const = default;
    int a;
};

int main() {
    Product* product = new Product(400);

    Wholesaler* wholesaler = new Wholesaler(product);

    Buyer* buyer = new Buyer(product);

    product->changePrice(320);
    product->changePrice(280);

    std::vector<X> v = { {10}, {9}, {11} }; // просто сортируем вектор структур
    SortDefaultComparator(v.begin(), v.end());

    return 0;
}

