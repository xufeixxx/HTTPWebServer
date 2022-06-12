#include"heaptimer.hpp"
#include"../Log/log.hpp"

void HeapTimer::SwapNode(heap_index idx1, heap_index idx2) {
    assert(idx1 >= 0 && idx1 < heap_.size());
    assert(idx2 >= 0 && idx2 < heap_.size());
    swap(heap_[idx1], heap_[idx2]);
    map_[heap_[idx1].sockfd] = idx1;
    map_[heap_[idx2].sockfd] = idx2;
}

heap_index HeapTimer::upshift(heap_index idx) {
    assert(idx >= 0 && idx < heap_.size());
    if (idx == 0)return idx;
    heap_index p_idx = (idx - 1) / 2;
    while (p_idx >= 0) {
        if (heap_[p_idx].expire_time < heap_[idx].expire_time)break;
        SwapNode(p_idx, idx);
        idx = p_idx;
        if (!idx)break;
        p_idx = (idx - 1) / 2;
    }
    return idx;
}

heap_index HeapTimer::downshift(heap_index idx) {
    assert(idx >= 0 && idx < heap_.size());
    if (idx >= (heap_.size() - 1) / 2 && idx < heap_.size())return idx;
    while (idx >= 0 && idx < (heap_.size() - 1) / 2) {
        heap_index left_c = 2 * idx + 1;
        heap_index right_c = 2 * idx + 2;
        if (left_c < heap_.size() && right_c < heap_.size()) {
            if (heap_[idx].expire_time <= heap_[left_c].expire_time && heap_[idx].expire_time <= heap_[right_c].expire_time) {
                break;
            }
            else {
                heap_index swap_idx = heap_[left_c].expire_time < heap_[right_c].expire_time ? left_c : right_c;
                SwapNode(idx, swap_idx);
                idx = swap_idx;
            }
        }
        else if (left_c < heap_.size() && right_c >= heap_.size()) {
            if (heap_[idx].expire_time <= heap_[left_c].expire_time)break;
            else {
                SwapNode(idx, left_c);
                idx = left_c;
            }
        }
    }
    return idx;
}

TimerNode& HeapTimer::top() {
    assert(empty() == false);
    return heap_[0];
}

void HeapTimer::push(TimerNode node) {
    heap_.push_back(node);
    map_[node.sockfd] = heap_.size() - 1;
    upshift(heap_.size() - 1);
}

void HeapTimer::pop() {
    assert(empty() == false);
    SwapNode(0, heap_.size() - 1);
    downshift(0);
    map_.erase(heap_[heap_.size() - 1].sockfd);
    heap_.pop_back();
}

bool HeapTimer::addExpireTime(heap_index idx, time_t newtime) {
    assert(idx >= 0 && idx < heap_.size());
    if (newtime > heap_[idx].expire_time) {
        heap_[idx].expire_time = newtime;
        downshift(idx);
    }
    return true;
}

heap_index HeapTimer::map_index(int sockfd){
    auto it = map_.find(sockfd);
    if(it == map_.end())
        return -1;
    return it->second;
}

void HeapTimer::tick(){
    if(empty())return;
    LOG_INFO("%s","timer tick\n");
    while(!empty()){
        TimerNode node = top();
        if(time(nullptr) > node.expire_time){
            node.cb_func_();
            pop();
        }else
            break;
    }
}

