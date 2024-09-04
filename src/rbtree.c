#include "rbtree.h"
// assert 남발한 것 같기도 -> 수요일에 코드 정리 좀 하자
#include  <assert.h>
#include <stdlib.h>
// 디버그용 나중에 삭제해 수빈아
#include <stdio.h>

// 매개변수 : X
// 리턴 값 : 생성한 rb트리 구조체의 포인터
// 테스트 코드의 검증 요소 : 리턴 값 NULL이어서는 안된다. 리턴 값의 루트 노드는 NULL이어야한다.
// 즉, root노드가 NULL인 rb트리 구조체를 반환해야한다!! 쉬작~
rbtree *new_rbtree(void) {
  // rbtree 구조체를 위한 메모리 동적 할당
  rbtree *p = (rbtree *)calloc(1, sizeof(rbtree));

  // 기존 코드 스타일 유지하면서 메모리 할당에 성공했는지 확인해주기
  assert(p != NULL);
  
  // rbtree 구조체의 root 포인터를 NULL로 설정 -> 트리가 비어있다는 뜻
  // 일단 nil 없이 구현해보고 하다가 중간에 어려우면 추가해볼게융
  p->root = NULL;
  
  return p;
}

// 트리의 노드들을 재귀로 들르면서 메모리 해제해준다.
void free_node(rbtree *t, node_t *n) {
  // 비어있는 트리의 경우 n은 NULL 일 수 있따
  assert(t != NULL);
  
  // 현재 노드가 NULL이 아닐 때! 자식들 재귀로 해제하고 현재 노드를 해제한다.
  if (n != NULL) {
    free_node(t, n->left);
    free_node(t, n->right);
    
    free(n);
  }
}

// 매개변수 : new_rbtree로 반환되었던 포인터의 주소
// 리턴값 : X
// 테스트 코드의 검증 요소 : valgrind로 확인해보았을 때 해제되지 않은 메모리가 나타나지 않아야한다.
// 인자로 주어진 트리가 사용했던 메모리 모두 free 해주기
void delete_rbtree(rbtree *t) {
  // dfs ㄱㄱ -> 새 함수 만들었뜸 (free_node)
  assert(t != NULL);
  // 트리의 노드들이 사용한 메모리 해제~
  free_node(t, t->root);
  // rb 트리 자체가 사용한 메모리도 해제!
  free(t);
}

// -------------------------------------
// 여기 아래부터 삽입 시 트리 조정을 위해 필요한 함수
// 왼쪽으로 회전시키는 함수
// 매개변수 : rbtree 구조체의 주소, 회전의 기준이 될 노드의 주소
// 리턴값 : X
// 기준 노드와 기준 노드의 오른쪽 자식의 위치 교환
// 오른쪽 자식의 왼쪽 자식을 기준노드의 오른쪽 자식으로 편입
void rotate_left(rbtree *t, node_t *target) {
  // 우리의 또 다른 타겟 -> 기준 노드의 오른쪽 자식
  node_t *target_right_child = target->right;
  // 자식 편입
  target->right = target_right_child->left;

  // 아까 편입한 자식의 부모 수정 (부모가 바뀌었으니께)
  if (target->right != NULL) {
    target->right->parent = target;
  }

  // 기준 노드의 부모를 오른쪽 자식의 부모로 바꾸깅
  target_right_child->parent = target->parent;
  
  // 만약 기준 노드가 루트 노드였으면
  if (target->parent == NULL) {
    t->root = target_right_child;
  } 
  // 부모님 있었을 때 (왼쪽 자식이었는지, 오른쪽 자식이었는지)
  else if (target == target->parent->left) {
    target->parent->left = target_right_child;
  } 
  else {
    target->parent->right = target_right_child;
  }
  
  // 기준 노드를 오른쪽 자식의 왼쪽 자식으로
  target_right_child->left = target;
  // 기준 노드의 부모를 오른쪽 자식 노드로~~
  target->parent = target_right_child;
}

// 오른쪽으로 회전시키는 함수 -> 왼쪽과 방향만 다르다~
// 매개변수 : rbtree 구조체의 주소, 회전의 기준이 될 노드의 주소
// 리턴값 : X
// 기준 노드와 기준 노드의 왼쪽 자식의 위치 교환
// 왼쪽 자식의 오른쪽 자식을 기준노드의 왼쪽 자식으로 편입
void rotate_right(rbtree *t, node_t *target) {
  node_t *target_left_child = target->left;

  target->left = target_left_child->right;
  
  if (target->left != NULL) {
    target->left->parent = target;
  }

  target_left_child->parent = target->parent;

  if (target->parent == NULL) {
    t->root = target_left_child;
  }
  else if (target == target->parent->left) {
    target->parent->left = target_left_child;
  } 
  else {
    target->parent->right = target_left_child;
  }

  target_left_child->right = target;
  target->parent = target_left_child;
}

// 매개변수 : rbtree 구조체의 주소, 삽입된 노드의 주소
// 리턴값 : X
// 테스트 코드의 검증 요소 : test_search_constraint에서 필요한 조건들 만족해야한다
// 필요한 조건 -> 이진 탐색 트리 조건 + rb 트리의 색깔 조건 5가지
// 삽입 후에도 rb트리 특성 만족하게 조절하는 함수
void adjust_tree(rbtree *t, node_t* new_node) {
  // 새로운 노드의 부모가 빨간색이면 바꿔줘야함!!! (왜냠 새로운 노드도 빨간색이니까)
  while (new_node->parent != NULL && new_node->parent->color == RBTREE_RED) {
    node_t *grandparent = new_node->parent->parent;
    node_t *uncle = (new_node->parent == grandparent->left) ? grandparent->right : grandparent->left;

    // case 1: 삽입 노드, 부모, 삼촌 빨강일 때!
    // -> 부모, 삼촌 검정으로 만들고, 할아버지 빨강으로 만든 다음에 체크!
    if (uncle != NULL && uncle->color == RBTREE_RED) {
      new_node->parent->color = RBTREE_BLACK;
      uncle->color = RBTREE_BLACK;
      grandparent->color = RBTREE_RED;
       // 할아버지 기준으로 rb트리 조건 만족하는지 다시 체크 -> while문
      new_node = grandparent;
    } 
    // case 2, case 3 드갈건데 -> 부모가 할아버지의 왼쪽자식인지 오른쪽 자식인지 일단 구분
    else {
      // 부모가 왼쪽 자식이었으면
      if (new_node->parent == grandparent->left) {
        // Case 2: 새로운 노드가 오른쪽 자식인 경우 -> 꺾어진 모양 -> case3로 바꿔줘야함
        // 부모기준 왼쪽 회전 & case3
        if (new_node == new_node->parent->right) {
          rotate_left(t, new_node->parent);
        }
        // Case 3: 새로운 노드가 왼쪽 자식인 경우 -> 일자 모양
        // 부모 색 검정으로, 할아버지 색 빨강으로, 할아버지 기준 오른쪽 회전
        new_node->parent->color = RBTREE_BLACK;
        grandparent->color = RBTREE_RED;
        rotate_right(t, grandparent);
      } 
      // 부모가 오른쪽 자식이었으면
      else {
        // Case 2: 새로운 노드가 왼쪽 자식인 경우 -> 꺾어진 모양 -> case3로 바꿔줘야함
        // 부모기준 오른쪽 회전 & case3
        if (new_node == new_node->parent->left) {
          rotate_right(t, new_node->parent);
        }
        // Case 3: 새로운 노드가 오른쪽 자식인 경우 -> 일자 모양
        // 부모 색 검정으로, 할아버지 색 빨강으로, 할아버지 기준 왼쪽 회전
        new_node->parent->color = RBTREE_BLACK;
        grandparent->color = RBTREE_RED;
        rotate_left(t, grandparent);
      }
    }
  }
  // 루트는 항상 검정색 유지하게하기~
  t->root->color = RBTREE_BLACK;
}
// 여기까지 삽입 시 트리 조정을 위해 필요한 함수
// -------------------------------------

// 매개변수 : new_rbtree로 반환되었던 포인터의 주소, key
// 루트 노드에 값 들어갔을 때는 left, right, parent NULL이어야한다.
// 노드 삽입하고 rb트리 균형 유지하도록 설정해주어야하겠찌요
node_t *rbtree_insert(rbtree *t, const key_t key) {
  assert(t != NULL);

  node_t* new_node = (node_t*)calloc(1, sizeof(node_t));
  // 메모리 할당 잘 됐는지 확인하고
  assert(new_node != NULL);

  // 삽입할 때 색은 무조건 빨강 -> 5번 조건때문에
  new_node->color = RBTREE_RED;
  new_node->key = key;
  new_node->left = NULL;
  new_node->right = NULL;

  // 빈 트리였으면 그냥 루트 노드에 때려박기
  if (t->root == NULL){
    t->root = new_node;

    // 삽입은 무조건 빨간색으로 시작하지만 루트 노드이기 때문에 일단 블랙으로 설정
    new_node-> color = RBTREE_BLACK;
    new_node->parent = NULL;
    
    // 루트 노드 삽입일 때는 트리 조절 과정 필요 X
    return new_node;
  }

  // 트리가 비어있지 않다면, 문제 발생 쫌 복잡합
  // 루트부터 숫자 비교하면서 적절한 자리에 삽입
  node_t* curr = t->root;

  while(1){
    // 삽입해야할 값이 현재 노드 보다 작으면->왼쪽 서브트리 탐색
    if (key < curr->key){
      // 왼쪽 노드 비어있으면 거기에 넣기!
      if (curr->left == NULL){
        curr->left = new_node;
        new_node->parent = curr;
        break;
      }
      // 왼쪽 노드 차 있으면 -> 왼쪽 서브트리 탐색
      else {
        curr = curr->left;
      }
    }
    // 삽입해야할 값이 현재 노드 보다 크거나 같으면 -> 오른 쪽 서브트리 탐색
    else {
      // 오른쪽 노드 비어있으면 거기 넣기!
      if(curr->right == NULL){
        curr->right = new_node;
        new_node->parent = curr;
        break;
      }
      // 오른쪽 노드 차 있으면 -> 오른쪽 서브트리 탐색
      else {
        curr = curr->right;
      }
    }
  }
  
  // 조건을 확인하는 추가 로직 구현하는 것보다 그냥 무조건 fix하는 함수 실행하는 것이 더 효율적이다.
  // rb트리 조건 만족하게 회전하고 어쩌구 하는 함수
  adjust_tree(t, new_node);

  return new_node;
}

// 매개변수 : new_rbtree로 반환되었던 포인터의 주소, 찾고자 하는 데이터
// 리턴값 : 키가 트리에 있으면 - 키를 가지고 있는 노드, 키가 트리에 없으면 - NULL
node_t *rbtree_find(const rbtree *t, const key_t key) {
  assert(t != NULL);
  
  // 루트 노드부터 탐색 시작!
  node_t* curr = t->root;

  while(curr != NULL){
    // 찾고자 하는 값이 현재 노드보다 작으면 -> 왼쪽 서브트리 탐색
    if (key < curr->key) {
      curr = curr->left;
    }
    // 찾고자 하는 값이 현재 노드보다 크면 -> 오른쪽 서브트리 탐색
    else if (key > curr->key) {
      curr = curr->right;
    }
    // key == curr->key
    else{
      // 키를 가지고 있는 노드의 포인터 리턴
      return curr;
    }
  }
  // 커서가 NULL이 될 동안 키 못 찾으면 NULL 반환
  return NULL;
}

// 매개변수 : new_rbtree로 반환되었던 포인터의 주소
// 리턴값 -> 최솟값을 key로 가지고 있는 노드의 포인터
// 이진 탐색 트리에서 최솟값 -> 맨 왼쪽 아래 노드
node_t *rbtree_min(const rbtree *t) {
  assert(t != NULL);

  node_t* minimum_node = t->root;
  
  // 트리 비어있을 때
  if (minimum_node == NULL) {
    return NULL;
  }
  
  // 왼쪽 맨 아래로 이동
  while (minimum_node->left != NULL) {
    minimum_node = minimum_node->left;
  }
  
  // 최솟값을 가진 노드의 포인터 반환
  return minimum_node;
}

// 매개변수 : new_rbtree로 반환되었던 포인터의 주소
// 리턴값 -> 최댓값을 key로 가지고 있는 노드의 포인터
// 이진 탐색 트리에서 최댓값 -> 맨 오른쪽 아래 노드
node_t *rbtree_max(const rbtree *t) {
  assert(t != NULL);

  node_t *maximum_node = t->root;

  // 트리 비어있을 때
  if (maximum_node == NULL) {
    return NULL;
  }

  // 오른쪽 맨 아래로 이동
  while (maximum_node->right != NULL) {
    maximum_node = maximum_node->right;
  }

  // 최솟값을 가진 노드의 포인터 반환
  return maximum_node;
}

// -------------------------------------
// 여기 아래부터 삭제 시 트리 조정을 위해 필요한 함수
// 삭제될 노드 대체 -> successor
// seccessor 대체 -> successor의 오른쪽 자식 (존재한다면!) 
// 매개변수 : rbtree 구조체의 주소, 삭제될 노드의 주소, successor의 주소
// 리턴값 : X

// 기준 노드와 기준 노드의 오른쪽 자식의 위치 교환
// 오른쪽 자식의 왼쪽 자식을 기준노드의 오른쪽 자식으로 편입
void transplant(rbtree *t, node_t *delete_node, node_t *successor) {
  // 삭제될 노드가 루트 노드였던 경우
  // rb트리의 루트노드를 successor로 교체해준다
  if (delete_node->parent == NULL) {
    t->root = successor;
  }
  // 삭제될 노드가 부모님이 있을때
  // 부모님의 왼쪽 자식일 때 -> successor로 왼쪽 자식 대체
  else if (delete_node == delete_node->parent->left) {
    delete_node->parent->left = successor;
  } 
  // 부모님의 오른쪽 자식일 때 -> succeessor로 오른쪽 자식 대체
  else {
    delete_node->parent->right = successor;
  }

  // successor가 NULL이 아니면, succesor의 부모 업데이트
  if (successor != NULL) {
    successor->parent = delete_node->parent;
  }
}

// 매개변수로 주어진 노드를 루트노드로 하는 서브트리의 최솟값 찾기 -> successor 찾기
node_t *subtree_min(node_t *node) {
  node_t* minimum_node = node;

  // 입력 노드가 NULL이면 NULL 반환
  if (minimum_node == NULL) {
    return NULL;
  }

  // 노드가 NULL이 아닐 때까지 왼쪽 자식 노드로 이동
  while (minimum_node->left != NULL) {
    minimum_node = minimum_node->left;
  }
  
  // 최솟값을 가진 노드의 포인터 반환
  return minimum_node;
}

// 삭제한 다음에 rb트리 특성 유지하게 조정해주는 함수
void delete_fixup(rbtree *t, node_t *x, node_t *x_parent, int x_position) {
  // 타겟이 루트노드가 아니고, NULL 이거나 색이 검정이면 ㄱ
  while ((x != t->root) && (x == NULL || x->color == RBTREE_BLACK)) {
    // 부모가 있으면 형제 노드 지정해준다
    node_t *sibling = NULL;
    if (x_parent != NULL) {
      sibling = (x_position == 0) ? x_parent->right : x_parent->left;
    }
    
    // 형제 노드 없으면 그냥 break
    if (sibling == NULL) {
      break;
    }
    
    // Case 1: 형제의 색이 빨간색이었으면
    // 부모랑 형제의 색을 교체하고 부모 기준으로 회전한 뒤 doubly black 기준 case 2,3,4로 해결한다
    if (sibling->color == RBTREE_RED) {
      sibling->color = RBTREE_BLACK;
      x_parent->color = RBTREE_RED;
      // x가 왼쪽 자녀였으면 부모 기준 왼쪽 회전
      if (x_position == 0) {
        rotate_left(t, x_parent);
        sibling = x_parent->right;
      } 
      // x가 오른쪽 자녀였으면 부모 기준 오른쪽 회전
      else {
        rotate_right(t, x_parent);
        sibling = x_parent->left;
      }
    }
    
    // Case 2: 형제 노드가 검정색이고, 두 자식도 검정색인 경우
    // 부모님에게 extra블랙을 전달한다
    if (sibling != NULL && (sibling->left == NULL || sibling->left->color == RBTREE_BLACK) && (sibling->right == NULL || sibling->right->color == RBTREE_BLACK)) {
      sibling->color = RBTREE_RED;
      x = x_parent;
      x_parent = x_parent->parent;
      
      if (x_parent != NULL) {
        x_position = (x == x_parent->left) ? 0 : 1;
      } 
      // 부모가 NULL이면 루프 종료
      else {
        break;
      }
    } 
    // 형제의 자식 중에 빨간 색이 있는 경우 -> Case 3 or Case 4
    else if (sibling != NULL) {

      // Case 3: 타겟이 왼쪽 자식이고 형제의 왼쪽 자식이 빨강
      // 형제의 자식과 형제의 색 바꾸고 형제 기준 회전 -> Case 4 실행
      if (x_position == 0 && (sibling->right == NULL || sibling->right->color == RBTREE_BLACK)) {
        if (sibling->left != NULL) sibling->left->color = RBTREE_BLACK;
        sibling->color = RBTREE_RED;
        rotate_right(t, sibling);
        sibling = x_parent->right;
      } 
      // Case 3: 타겟이 오른쪽 자식이고 형제의 오른쪽 자식이 빨강
      // 형제의 자식과 형제의 색 바꾸고 형제 기준 회전 -> Case 4 실행
      else if (x_position == 1 && (sibling->left == NULL || sibling->left->color == RBTREE_BLACK)) {
        if (sibling->right != NULL) sibling->right->color = RBTREE_BLACK;
        sibling->color = RBTREE_RED;
        rotate_left(t, sibling);
        sibling = x_parent->left;
      }
      
      // Case 4: 형제 검정 자녀 빨강인데 일자!
      // 형제는 부모색으로 바꾸고 자식, 부모 검정으로 해준 뒤 부모 기준 회전
      if (sibling != NULL) {
        sibling->color = x_parent->color;
        x_parent->color = RBTREE_BLACK;
        if (x_position == 0) {
          if (sibling->right != NULL) sibling->right->color = RBTREE_BLACK;
          rotate_left(t, x_parent);
        } 
        else {
          if (sibling->left != NULL) sibling->left->color = RBTREE_BLACK;
          rotate_right(t, x_parent);
        }
        // x를 루트로 함 -> while문 탈출 가능
        x = t->root;
      }
    }
  }
  if (x != NULL) x->color = RBTREE_BLACK; // x가 NULL이 아니면 색을 검정으로 설정
}
// 여기까지 삭제 시 트리 조정을 위해 필요한 함수
// -------------------------------------

// 매개변수 : new_rbtree로 반환되었던 포인터의 주소, 삭제할 노드의 주소
// 리턴값 : 따로 없고 잘 완료 됐다는 거 표시해주기 위해 return 0
// 테스트 코드의 검증 요소 : erase_root -> rb트리 루트 비어있는지
// find_erase -> 삭제하고 트리 잘 비워지는지
int rbtree_erase(rbtree *t, node_t *delete_node) {
  // 삭제될 노드의 자식 0개나 1개면 얘가 삭제될 노드 대체
  node_t *new_child = NULL;
  // 삭제될 노드의 자식 2개면 successor가 삭제될 노드 대체
  node_t *successor = NULL;
  // 삭제될 색상!
  color_t delete_color;
  node_t *parent = delete_node->parent;
  // 삭제 후 rb트리 조정할 때 형제 노드 찾을 때 씀
  // 왼쪽 자식 -> 0 오른쪽 자식 -> 1
  int position = (parent != NULL && delete_node == parent->left) ? 0 : 1;
  
  // 삭제될 노드의 자식이 0개 혹은 1개 -> 삭제될 색은 삭제될 노드의 색
  // 삭제될 노드의 자리는 NULL 또는 1개인 자식이 대체하면된다.
  if (delete_node->left == NULL || delete_node->right == NULL) {
    new_child = (delete_node->left != NULL) ? delete_node->left : delete_node->right;
    delete_color = delete_node->color;
    transplant(t, delete_node, new_child);
  } 

  // 삭제될 노드의 자식이 2개 -> 삭제될 색은 successor의 색
  // 삭제될 노드의 자리는 successor가 채우고 successor의 자리는 successor의 오른쪽 자식 혹은 NULL이 채운다.
  else {
    successor = subtree_min(delete_node->right);
    delete_color = successor->color;
    new_child = successor->right;
    
    // successor가 삭제될 노드의 바로 아래 자식이 아니면
    if (successor->parent != delete_node) {
      // successor의 자리는 successor의 오른쪽 자식 혹은 NULL이 대신
      transplant(t, successor, successor->right);
      successor->right = delete_node->right;
      successor->right->parent = successor; 
      // 삭제될 노드의 자리는 successor가 대신
      transplant(t, delete_node, successor);
    } 
    // successor가 삭제될 노드의 바로 아래 자식이었으면
    // 그냥 삭제 노드 자리에 successor 두기 
    else {
      transplant(t, delete_node, successor);
    }
    // 자식에 대한 처리 ㄱㄱ
    successor->left = delete_node->left;
    successor->left->parent = successor;
    successor->color = delete_node->color;
    parent = successor;
    position = 1;
  }

  // 삭제될 색이 빨강이면 문제 없는데 검정이었으면 문제발생 -> 수정해줘야함
  if (delete_color == RBTREE_BLACK) {
    delete_fixup(t, new_child, parent, position);
  }

  // 할 일 다했으니까 이제 삭제할 노드 메모리 해제 ㄱㄱ
  free(delete_node);
  return 0;
}

// 중위 순회 하면서 매개변수로 주어진 배열에 오름차순으로 key를 저장
// 이중 포인터로 배열의 주소 +1씩 하면서 배열 채워주기
void inorder(node_t* root, key_t **arr) {
  if (root != NULL) {
    // 왼쪽 서브트리 순회
    inorder(root->left, arr);

    // 현재 노드의 키를 배열에 저장하고, 배열 포인터 +1 해서 다음 배열 주소로 넘어가기
    **arr = root->key;
    (*arr)++;

    // 오른쪽 서브트리 순회
    inorder(root->right, arr);
  }
}

// 매개변수 : new_rbtree로 반환되었던 포인터의 주소, 배열의 첫 요소 주소, 배열의 크기
// 리턴값 -> 따로 없고 잘 완료 됐다는 거 표시해주기 위해 return 0
// 이진 탐색트리 배열로 저장해줄 때 오름차 순으로 정렬되어야함
// 이진 탐색트리의 노드들 중위 순회 하면 된다!
int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n) {
  assert(t != NULL);

  // 중위 순회하세용~
  inorder(t->root, &arr);

  return 0;
}