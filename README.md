# Ray-Tracing #
## 题目 ##
基于闫令琪老师GAMES101课程作业7的扩展。</br>
在完成课程内关于表面渲染的作业基础上，进一步增加了体积散射。
## 实现 ##
沿用课程提供的P7框架，与此前作业中已实现的Ray Tracing。</br>修改了路径追踪器Scene::castRay()。</br>
首先仍通过采样计算直接光照，加入计算光在传播过程中的损失，消光系数κ。</br>
随机生成概率密度函数符合 $$p(s) = \kappa e^{-\kappa s} $$
的变量s，代表本次采样得到的光传播距离.</br>
计算光沿此方向首次碰撞物体经过的距离，和采样距离比较，判断光是能够打到物体表面，还是进入介质中。</br></br>
1. 如果光打到物体表面，那么非直接光照和表面散射计算方式相同：</br>取在半球上均匀分布的BRDF，对其进行一次采样得到反射方向，向该方向投射新射线。利用Russian Roulette决定何时停止递归。</br>
2. 如果光进入介质中，首先有一定概率被吸收。
	

- 设定一个散射概率，如果生成的(0,1)范围随机数大于该概率，则被吸收，返回间接光照(0,0,0)。</br>
- 如果被散射，则对散射过程的相位角采样。生成概率密度函数满足Lorenz-Mie散射方程$$ p_{MH}(x, \theta) = \frac{1}{4 \pi}(\frac{1}{2}+\frac{9}{2}(\frac{1+cos \theta}{2})^{8}) $$的夹角θ。再采样σ。计算出新方向，调用本函数投射光线。
- 根据 $$ L(y, \theta) = \int_{\Omega}\frac{\sigma}{\kappa}f(y,\theta',\theta)L(y,\theta')d\omega' $$ 原理，利用返回值计算得到间接光照。


</br></br>将直接光照和间接光照相加等于该位置最终光照数据。
## 结果 ##
下图展示了原P7中实现的表面散射（左）和现加入体积散射（右）的效果对比。</br>
![avatar](https://github.com/seeeagull/Ray-Tracing/blob/main/images/contrast.PNG)</br>

## 参考资料 ##
Wojciech Jarosz. Efficient Monte Carlo Methods for Light Transport in Scattering Media. Ph.D. dissertation, UC San Diego, September 2008.</br></br>
Lafortune E.P., Willems Y.D. (1996) Rendering Participating Media with Bidirectional Path Tracing. In: Pueyo X., Schröder P. (eds) Rendering Techniques ’96. EGSR 1996. Eurographics. Springer, Vienna. https://doi.org/10.1007/978-3-7091-7484-5_10
