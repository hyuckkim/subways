# subways
2026-1 기말

## 목표
`[출발역] [도착역] [현재:시간]` 입력 시 타야 하는 열차의 종착역과 정차시간을 출력하는 CLI 프로그램  

### 예시 (실제데이터가아님, 예시임)
```
$ subways 정왕 인하대 17:19
오이도 17:19 K1234
인천 17:22 K1345
```
오이도에서 출발하는 사이 열차가 있어서 인식됨

```
$ subways 정왕 인하대 17:20
인천 18:02 K5678
```
오이도에서 출발하는 사이 열차가 없으므로 가만히 있는 것을 추천

## 사양
C with only standard library.

## 데이터
* /res/\[station\]
2026-06-05 시간표 수동 크롤링.
파일 이름은 [역이름].txt 형식으로 저장됨
(주의: 띄어쓰기가 없음)
출처: [레일.블루](https://rail.blue/railroad/logis/metrotimetable.aspx)

```
[역 이름] [일자] 시간표

[K1234 열차번호][도착:시간][종착역] 반복
[시간]
[종착역][도착:시간][K5678 열차번호] 반복
```

* roads.txt
종착역별로 노선을 정리해둠
`[종착역]; [...노선] 반복`
형식으로 이루어져 있음

* alias.txt
이름은 다르지만 같은 역인 경우를 정리해둠
`[대표이름]; [...보조이름] 반복`
형식으로 이루어져 있음

## 자료형
### 역 그래프 (Station Graph)

역 간 연결 관계만 표현 (시간 없음)
각 역은 “이웃 역 목록”을 가진다.
```
StationNode
- station name
- adjacent stations (연결된 역들)
```
roads.txt를 기반으로 생성
“어디로 이동 가능한지”만 표현
방향 노선(종착역 기준 순서)을 포함


### 노선 (Route)

하나의 종착역 기준으로 정렬된 역 리스트
여기서 종착역이라는 건 실제 역 이 아니라 일종의 운행 계통
(동인천급) 처럼.

```
Route
- terminal (종착역)
- ordered stations (운행 순서)
```

열차가 실제로 이동하는 순서 정의
환승 없이 “직선 이동 경로” 생성 가능
roads.txt의 핵심 구조

### 시간표 (Station Schedule)

각 역에 도착/출발하는 열차 정보
```
StationSchedule
- station name
- list of departures
Departure
Departure
- train number
- time
- terminal (종착역)
```
“이 역에서 언제 어떤 열차를 탈 수 있는가”
탐색의 시작점

### 탐색 상태 (Search State)

알고리즘이 실제로 움직이는 단위
```
State
- current station
- current time
```
Dijkstra / BFS의 노드 역할
“지금 어디에 언제 도착했는가”


### 상태 전이 (Transition)

두 가지 이동만 존재:

(1) 대기 (Wait)
같은 역에서 시간 증가
(2) 탑승 (Ride)
Departure → Train → 다음 역 도착

### 전체 시스템 구조
roads.txt
    ↓
Station Graph (이동 가능성)
    ↓
Station Schedule (출발 가능 열차)
    ↓
Train Paths (열차 이동 경로 복원)
    ↓
Search State Graph (시간 기반)
    ↓
Shortest Arrival Result

## 로직 (Abstract Algorithm)
### 입력 처리

사용자로부터 [출발역] [도착역] [현재시간]을 입력받음.

alias.txt를 참조해 동일 역 이름을 통합.

시간을 HH:MM → 분 단위 정수로 변환.

### 그래프 생성

roads.txt를 읽어 Station Graph 생성.

각 종착역 기준으로 Route를 만들고, 역 순서를 정렬.

출발역에서 도착역까지 reachable set을 계산해 불필요한 역 제거.

### 열차 경로 복원

같은 열차번호를 여러 역에서 찾아 Train Path를 구성.

열차별로 전체 운행 경로와 도착 시각을 정리.

### 탐색 초기화

State(start station, current time)을 생성.
best_arrival_time[station] = (∞, 열차번호 null)로 초기화 후, 출발역은 현재시간으로 설정.
경로를 저장하기 위해 best_arrival_time에 경로를 같이 저장

우선순위 큐(현재시간 기준)에 (출발역, 현재시간) 삽입.

### 탐색 알고리즘 (시간 기반 Dijkstra)  
큐에서 (station, time)을 꺼냄.
탑승(Ride): 현재 시간 이후 출발 가능한 열차를 찾아 (파일 읽기, 캐싱)
 그 열차들의 모든 다음 정차역을 큐에 삽입 (도착시간 2시간 이내의 모든 열차에 대해서 수행, 이미 큐에 해당 station이 더 이른 시간으로 존재한다면 pass)
 큐에 도착시간을 삽입하므로 역 파일을 파싱할 필요가 있음

### 종료 조건

도착역에 도달하면 탐색 종료.
best_arrival_time[destination]이 최단 도착 시각.
해당 경로를 추적해 결과 출력.

### 출력
최종적으로 "어떤 열차를 타야 하는지, 종착역과 출발/도착 시각"을 CLI에 표시.

## 예외 처리
역 이름에 맞는 역 파일이 없음 -> 경고
종착역에 맞는 노선 정보가  roads.txt가 없음 -> 경고
'회송' 은 노선이 아니므로 무시
늦은 열차의 시간대가 00시를 넘어간다면 24시간을 더해야 함.