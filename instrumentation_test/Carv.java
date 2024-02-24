import java.util.ArrayList;
import java.util.concurrent.locks.ReentrantLock;
class Foo {
      private ReentrantLock mutex = new ReentrantLock();
      public String a = "hello";
//    public volatile ArrayList<String> list = new ArrayList<String>();
      public void test() {

	mutex.lock();
        this.a += "\n";
        String test = this.a;

        System.out.println("test-.-");
	mutex.unlock();
     }
}
public class Carv {
        public static void main(String[] args) {
        Foo c = new Foo();
    /*	for (int i = 0; i < 10; i++) {
                    String b = c.a;
                    System.out.println(b);
            }
    */
    //	ArrayList<String> t = new ArrayList<>();

    //	t.add(c.a);

    //	c.test();

            //System.out.println(b);

        Thread t1 = new Thread(new Runnable() {
            public void run() {
                c.test();
            }
        });

        Thread t2 = new Thread(new Runnable() {
            public void run() {
                c.test();
            }
        });

        t1.start();
        t2.start();

        try {
            t1.join();
            t2.join();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
}
