# Volume Rendering Using Ray casting
-------------------------------------
MRI나 CT의 밀도 샘플링 기계를 이용하여 얻은 볼륨 데이터를 3D 가시화 하는 OpenGL 프로그램입니다.  
데모 영상은 [여기](https://www.youtube.com/watch?v=JTLe6dv1wfQ) 에서 확인하실 있습니다.  
볼륨 렌더링은 샘플링 격자점마다 주어진 Intensity 값을 통해 Transfer Function에 매칭된 색상으로  
가시화 해주는 렌더링 기법입니다.  

자세한 내용은 [GPU gems](https://developer.download.nvidia.com/books/HTML/gpugems/gpugems_ch39.html) 을 참조하시면 좋습니다.  

위 프로그램은 레이캐스팅을 이용하여 2 패스 렌더링을 하여 볼륨 데이터를 가시화합니다.  
첫번째 렌더링 패스에서는 미리 생성해둔 백 프레임버퍼(2D 텍스쳐를 생성해 바인드)에 큐브의 *뒷면*을 렌더링하고  
*해당 프래그먼트의 월드 스페이스 좌표*를 색상값으로 2D 텍스쳐에 저장합니다. 
두번째 렌더링 패스에서는 레이 캐스팅을 실행합니다.  
실제 화면에 출력할 프레임버퍼를 사용하며 이곳에는 첫번째 렌더링 패스를 통해 형성된 2D 텍스쳐의  
각 픽셀의 색상값(좌표값)을 읽어옵니다.  
위에 언급하였듯 각각의 프래그먼트들은 해당 프래그먼트의 위치 좌표를 색상으로 하여 저장을 해두었기 때문에 텍스쳐의 rgb 값을 읽어와  
좌표로 사용하여 빛의 진행 방향을 결정합니다.  
이 빛의 진행 방향으 나타내는 노멀 벡터를 구하고 이것에 샘플링 레이트를 곱하여 스텝당 빛의 진행 거리로 설정하고  
3D 텍스쳐로 전달한 볼륨 데이터 값(Intensity) 을 탐색하여 tff 테이블의 색상값을 얻어오고 현재 프래그먼트의 최종 색상값과 알파값을  
구합니다.   

# 결과물
------------
![result1.png](https://github.com/elensar92/Volume-Rendering/blob/master/result1.png)
![result2.png](https://github.com/elensar92/Volume-Rendering/blob/master/result2.png)


# Enviroment
______________
* Mac OS X 10.13.6 이상
* OpenGL 4.1 이상  

# Dependencies
_______________
* OpenGL 4.1 
* GLFW 3.3  
* GLM  
