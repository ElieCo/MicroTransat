

template <class T1, class T2>

class Map {

public:
  Map(){
    // Initialize the first element at NULL.
    m_first_element = NULL;
  }

  ~Map(){
    clear();
  }

  /**
   * Set this key at this value.
   * Create the element if it doesn't already exists.
   * @param key
   * @param value
   */
  void set(T1 key, T2 value){
    Element* element = findElement(key);
    if (element) element->value = value;
    else initElement(key, value);
  }

  /**
   * Get the value of this key.
   * @param key
   * @return value
   */
  T2 get(T1 key){
    Element* element = findElement(key);
    if (element) return element->value;
    else return T2();
  }

  /**
   * Get the key at this place in the list.
   * @param place in the map
   * @return key
   */
  T1 keyAt(int place){
    if (place >= size()) return T1();
    Element* element = recursiveAt(m_first_element, place);
    return element->key;
  }

  /**
   * Get the value at this place in the list.
   * @param place in the map
   * @return value
   */
  T2 valueAt(int place){
    if (place >= size()) return T2();
    Element* element = recursiveAt(m_first_element, place);
    return element->value;
  }

  /**
   * Remove the element with this key.
   * @param key
   */
  void remove(T1 key){
    Element* element = findElement(key);
    
    Element* prev = element->prev_element;
    Element* next = element->next_element;

    prev->next_element = next;
    next->prev_element = prev;

    if (element == m_first_element){
      if (next == element) m_first_element = NULL;
      else m_first_element = next;
    }

    delete element;
  }

  /**
   * Delete all the elements.
   */
  void clear(){
    recursiveClear(m_first_element);
    m_first_element = NULL;
  }

  /**
   * Check if this key exists in the map.
   * @param key
   * @return true if this key exists, false if not.
   */
  bool contains(T1 key){
    if (findElement(key)) return true;
    else return false;
  }

  /**
   * Get the number of elements in he map.
   * @return the size of the map.
   */
  int size(){
    int count = 0;
    recursiveCount(m_first_element, count);
    return count;
  }

private:
  
  struct Element{
    T1 key;
    T2 value;
    Element* next_element;
    Element* prev_element;
  };

  /**
   * Create an element and put it in the list.
   */
  void initElement(T1 key, T2 value){
    
    Element* element = new Element();
    
    element->key = key;
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

  Element* findElement(T1 key){
    return recursiveFind(key, m_first_element);
  }

  Element* recursiveFind(T1 key, Element* element){
    if(DEBUGLEVEL >= 2) print("recursiveFind", key);
    if (!element) return NULL;
    if (element->key == key) return element;
    else {
      if (element->next_element == m_first_element) return NULL;
      else return recursiveFind(key, element->next_element);
    }
  }

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

