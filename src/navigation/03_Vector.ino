
template <class T>

class Vector{

  public:

  Vector(){
    // Initialize the first element at NULL.
    m_first_element = NULL;
  }

  ~Vector(){
    clear();
  }

  /**
   * Add an element in the vector.
   * @param value of this element.
   */
  void push_back(T value){
    Element* element = new Element();

    element->value = value;

    if(!m_first_element){
      m_first_element = element;
      m_first_element->next_element = m_first_element;
      m_first_element->prev_element = m_first_element;
    }
    else {
      Element* last = m_first_element->prev_element;
      last->next_element = element;
      element->prev_element = last;
      element-> next_element = m_first_element;
      m_first_element->prev_element = element;
    }
  }

  /**
   * Add an element in the vector at the place i.
   * @param i the place and value of this element.
   */
  void insert(int i, T value){
    if (i >= size())
      return push_back(value);

    Element* element = new Element();
    element->value = value;

    // Can not be a first element because if it is we do a push_back
    // Insert element between prev and next.
    Element* prev = elementAt(i-1);
    Element* next = elementAt(i);
    prev->next_element = element;
    element->next_element = next;
    next->prev_element = element;
    element->prev_element = prev;

    if (i == 0)
      m_first_element = element;

  }

  /**
   * remove the element in the vector at the place i.
   * @param i the place.
   */
  void removeAt(int i){
    if (size() <= 1)
      return clear();

    Element* second_element = m_first_element->next_element;

    // Make link between prev and next element and then delete the element
    Element* prev = elementAt(i-1);
    Element* element = elementAt(i);
    Element* next = elementAt(i+1);
    prev->next_element = next;
    next->prev_element = prev;
    delete element;

    if (i == 0)
      m_first_element = second_element;
  }

  /**
   * Get the number of elements in the vector.
   * @return the size of the vector.
   */
  int size(){
    int count = 0;
    recursiveCount(m_first_element, count);
    return count;
  }

  /**
   * Get the value at this place in the list.
   * @param place in the vector
   * @return value
   */
  T at(int place){
    int s = size();
    if (s <= 0) return T();
    while(place < 0) place += s;
    while(place >= s) place -= s;
    Element* element = recursiveAt(m_first_element, place);
    return element->value;
  }

  /**
   * Get a pointer on the value at this place in the list.
   * @param place in the vector
   * @return value*
   */
  T* ptrAt(int place){
    int s = size();
    if (s <= 0) return NULL;
    while(place < 0) place += s;
    while(place >= s) place -= s;
    Element* element = recursiveAt(m_first_element, place);
    return &(element->value);
  }

  /**
   * Delete all the elements.
   */
  void clear(){
    recursiveClear(m_first_element);
    m_first_element = NULL;
  }

  private:

  struct Element{
    T value;
    Element* next_element;
    Element* prev_element;
  };

  Element* elementAt(int i){
    int s = size();
    while(i < 0) i += s;
    while(i >= s) i -= s;
    return recursiveAt(m_first_element, i);
  }

  void recursiveClear(Element* element){
    if (!element) return;
    if (element->next_element != m_first_element)
      recursiveClear(element->next_element);
    delete element;
  }

  void recursiveCount(Element* element, int &count){
    if (!element) return;
    if (element->next_element != m_first_element)
      recursiveCount(element->next_element, count);
    count++;
  }

  Element* recursiveAt(Element* element, int &count){
    if (!element) return NULL;
    if (count > 0) {
      count--;
      return recursiveAt(element->next_element, count);
    } else
      return element;
  }

  Element* m_first_element;

};
