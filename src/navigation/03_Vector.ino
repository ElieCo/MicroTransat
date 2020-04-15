
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
      last-> next_element = element;
      element->prev_element = last;
      element-> next_element = m_first_element;
      m_first_element->prev_element = element;
    }
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
    if (place >= size()) return T();
    Element* element = recursiveAt(m_first_element, place);
    return element->value;
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

  void recursiveClear(Element* element){
    if(DEBUGLEVEL >= 2) print("recursiveClear");
    if (!element) return;
    if (element->next_element != m_first_element)
      recursiveClear(element->next_element);
    delete element;
  }

  void recursiveCount(Element* element, int &count){
    if(DEBUGLEVEL >= 2) print("recursiveCount", count);
    if (!element) return;
    if (element->next_element != m_first_element)
      recursiveCount(element->next_element, count);
    count++;
  }

  Element* recursiveAt(Element* element, int &count){
    if(DEBUGLEVEL >= 2) print("recursiveAt", count);
    if (!element) return NULL;
    if (count > 0) {
      count--;
      return recursiveAt(element->next_element, count);
    } else
      return element;
  }

  Element* m_first_element;
  
};

