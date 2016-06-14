= インターネットの1秒がもし1年だったら(仮)

//raw[|latex|\\chapterauthor{hakatashi・Mine}]

//lead{

//}

== はじめに

たぶんはじめまして。博多市(@hakatashi)です。
今回は技術書典向けの小企画として、
「インターネットの1秒がもし1年だったら」という
出落ち極まりない記事を書こうと思います。

インターネットというのは光の速さを身をもって感じることができるメディアです。
先日PHPのパッケージマネージャーであるcomposerを高速化するという内容のスライド@<fn>{composer}を
拝見したのですが、そこではcomposerが遅い原因として「光の速さが遅すぎる」というものが挙げられていました。

ユニークな考え方ですが言われてればたしかに道理で、日本からアメリカのサーバーまでハンドシェイクで
何度も往復していると、確かに光といえど100ミリ秒単位で時間を浪費しています。
サーバーが近ければ通信は早いというのは誰もが知っていることですが、
光の速度のせいだと言われるとなにやら圧倒されるものがあります。

この記事では、そんなネットワークの微視的な時間スケールについて徹底的に解剖します。
一回の通信を解析し、それぞれ時系列順に細かく分解し、それぞれの操作でどれくらいの時間が費やされ、
どんなイベントがいつ発生するのかを逐一追っていきたいと思います。

とはいえ、ネットワーク通信におけるタイムスパンはミリ秒単位で数えられます。
今回は、そんな微細な時間の移り変わりをなるべくわかりやすくするため、
@<strong>{通信上の1秒という時間を1年にまで引き伸ばし}、@<strong>{約3000万倍}の時間スケールで
ネットワークのイベントを追跡してみます。

//footnote[composer][@<href>{http://www.slideshare.net/hinakano/composer-phpstudy}]

=== 対象読者

 * ほげ
 * ほげ

=== 共著者について

=== シチュエーション

では、実際にどんな通信を解析するとよいでしょう。
徹底解剖する通信として、なるべく我々が普段慣れ親しんでいて、
それでいてそれなりに複雑で解剖しごたえのある通信を採用したいところです。

そこで、今回は@<strong>{HTTPSプロトコル}の通信をメインに解析しました。
ふだん我々がブラウザから毎日使っているプロトコルなのは言うまでもなく、
SSL/TLSの処理を挟むので、それなりに面白い結果になるのではないでしょうか。

また、HTTPS通信を行う際に必要となるARPやDNS通信についても述べていきます。

さらに、通信を行うサーバーとクライアントの場所ですが、
今回の技術書典の会場でもある東京のインターネット環境から、
さくらインターネットが誇る、石狩データセンターまで通信を行います。

Googleマップによると、東京から、石狩データセンターがある石狩市新港中央までは1177.3km。
光ファイバーに用いられる石英ガラスの屈折率は1.50程度@<fn>{silicon}なので、光は約20万km/sでこの距離を移動します。
つまり、東京から石狩までの物理的な片道時間は約5.9ミリ秒となります。
瞬く間もないほどの時間ですが、時間スケールを1年に引き伸ばすと2日と3時間半もかかります。
往復で4日と7時間。石狩までぶらり旅といった感じですね。

//footnote[silicon][光学ガラス材料 - シグマ光機株式会社 @<href>{http://www.products-sigmakoki.com/category/opt_d/opt_d01.html}]

==== 1秒→1年

1秒が1年になった世界の様子をもう少し見てみましょう。

真空中の光は時速34kmで移動します。
先ほど出てきた光ファイバー中の光速は時速23km程度になるので、
自転車か、休憩しながらのドライブ旅程度の速さになります。
ちなみに男子マラソンの世界記録は時速20.6kmです。@<fn>{marathon}@<fn>{relativity}

3GHzのCPUは、こんな世界でも1秒間に95回という高橋名人の6倍のクロックを刻みます。
PC3-12800のメモリーの最大転送速度は、
もはやダイアルアップ接続よりも遥かにナローバンドですが、
1秒間に3200ビットの情報を読み出すことができます。

//footnote[marathon][2014年ベルリンマラソンのデニス・キメット選手の記録、2時間2分57秒より。]
//footnote[relativity][ただし、この速度で走行するとこの世界では体重が26%増えるのでオススメしない。]

==== 制約など

今回パケットを解析する上で、話を簡便化するためにいくつかの制約を加えています。

 * DNSのシステムを明確に示すため、ローカルホストにDNSサーバーを置き、
   そこからリクエストのたびにルートDNSサーバーからアドレスを引いています。
 * 時間がかかりすぎるため、DNSSECの検証は省略しています。
 * SSL/TLSの証明書検証は省略しています。

=== 測定環境

測定には、ラップトップマシン上のUbuntuの仮想環境を使用しました。
一回の測定ごとにDNSキャッシュとARPキャッシュをクリアし、
なるべくクリーンな状態で測定を行いました。

また、先程も述べたとおりDNSサーバーはローカルホストに設置し、
そこから外部に向けてアドレスを引くようにしました。

...

== ルーターのアドレス解決

=== 1月1日 午前0時0分 ARPリクエスト送信

あけましておめでとうございます。NHKの「ゆく年くる年」を見ながらインターネットの年が明けます。

この瞬間、東京でまったりと紅白歌合戦を見ていたクライアントちゃんは、
石狩にいるサーバーちゃんに聞きたいことがあるのを思い出しました。
長い長い通信の始まりです。

さっそくクライアントちゃんはサーバーちゃんに手紙を書くことにしました。

クライアントちゃんは何もわかりません。
郵便ポスト(ルーター)の場所も、サーバーちゃんの住所(IPアドレス)も、
いつも使っていた住所録(DNSサーバー)の場所も覚えていません。
キャッシュを削除したクライアントちゃんは記憶を失ってしまったのです。

覚えているのは、ただ石狩にいるはずのサーバーちゃんのことだけ……。

困ったクライアントちゃんは、まずは郵便ポスト(ルーター)を探すことにしました。
家の中や向こう三軒両隣のことならともかく、遠く石狩に住むサーバーちゃんに手紙を届けるには、
何よりポストがなければ話が始まりません。

クライアントちゃんはもうネットワークに接続しているので、
ルーターのIPアドレスはすでに分かっています。
しかし実際にルーターと通信するには、ルーターのMACアドレスが必要です。
これを引くために、@<kw>{ARP, Address Resolution Protocol}と呼ばれる
プロトコルを用います。

ARPリクエストはIPアドレスに対応するMACアドレスを検索するリクエストです。
MACアドレスの@<kw>{プロードキャストアドレス}(FF:FF:FF:FF:FF:FF)に向けて発信することで、
同一ネットワーク内のすべての機器にこのリクエストを送信することができます。

クライアントちゃんは街中に響く声でポストの場所を尋ねました。

=== 1月1日 午前3時26分 ARPレスポンス受信

いったい今何時だと思っているんでしょうか。
クライアントちゃんの声は街中に響き渡り、近隣住民からたいへん大目玉を食らいましたが、
努力の甲斐あって、3時間半後に親切な人がポストの場所を教えてくれました。

ARPは、ブロードキャストでネットワーク全体に配信されたイーサネットフレームに対して、
尋ねられているIPアドレスが自分のものであればMACアドレスを応答するという
シンプルな仕組みで動作します。

今回は事前にARPキャッシュを削除してから通信を行ったので、
通信前に必ずARPの問い合わせが走るようにして計測したのですが、
そもそもARPは頻繁にキャッシュを消去します。@<fn>{arpcache}
クライアントちゃんはとても忘れっぽいのです。

//footnote[arpcache][Ubuntuの場合、キャッシュの有効時間は15分程度。]

== DNS解決

=== 1月1日 午前5時7分 DNSクエリ送信(1回目)

親切なおじさんのおかげで、クライアントちゃんは郵便ポストの場所を知ることができました。
またすぐ忘れてしまうのですが、この世界では約900年後のことなので、特に気にすることはありません。

となると次にクライアントちゃんが知るべきはサーバーちゃんの住所(=IPアドレス)です。
郵便ポストを見つけても相手の住所がわからないと手紙は送れません。
サーバーちゃんの住所を知るために、クライアントちゃんは
ルートサーバーに住所を問い合わせることにしました。

電話番号に電話番号問い合わせサービスがあるように、ネットワークの世界にも、
ドメイン名から相手のアドレスを問い合わせるための@<kw>{DNS, Domain Name Service}があります。

電話番号の場合は104番という番号を知っていれば他の番号を問い合わせることができます。
インターネットにおける“104”は、世界に13個存在する@<kw>{ルートサーバー}のアドレスです。
今回は、13個の中で唯一日本の団体が管理している、Mルートサーバーに問い合わせるように
ルートサーバーの設定を変更しています。@<fn>{dnscluster}

サーバーちゃんの住所を一秒でも早く正確に知りたいクライアントちゃんは、
世界中に数あるDNSサーバーの中でも最も権威あるルートサーバーに一筆したためることにしました。
あらゆるキャッシュを削除したクライアントちゃんも、一番大事なルートサーバーの住所は覚えています。
あれこれ悩んで手紙を書き、ようやく完成したのはそれから1時間半後のことでした。
早朝の冷たい冬風が骨身に染みます。クライアントちゃんはコートを厚めに羽織って
ポストに手紙を投函しに行きました。

一刻も早く返事が帰ってくることを願って……。

//footnote[dnscluster][もっとも、ルートサーバーはクラスタ構成になっており国単位で分散しているため、必ずしも問い合わせたマシンが日本に存在するかどうかは保証されないのですが……。]

=== 1月14日 午前3時57分 DNSレスポンス受信(1回目)

最初に手紙を送ってから2週間が経過しました。
もうこの時点で直接会いに行ったほうが手っ取り早いんじゃないかという気がしますが、
石狩は遠いのです。そんな気軽に会いに行けないのです。たぶん。

ルートサーバーからようやく返事が帰ってきました。
郵便局が怠慢なのかルートサーバーがお役所仕事をしてるのか知りませんが、
とにかくこれでサーバーちゃんの住所を知ることができた……というわけではありません。

ルートサーバーは世界中のすべてのマシンのIPアドレスを記録しているわけではありません。
ドメイン名を問い合わせられたDNSサーバーは、自らが委任するDNSゾーンのネームサーバーの情報を返します。
今回問い合わせたドメイン名はさくらのVPSサーバーにデフォルトで割り当てられたドメイン@<fn>{sakuradomain}なので、
ルートサーバーは.jpサブドメイン@<fn>{subdomain}のネームサーバーを返してきました。

//footnote[sakuradomain][.vs.sakura.ne.jpで終わるドメイン名]
//footnote[subdomain][.jpはTLD(Top Level Domain)なのでサブドメインと呼んでいいか微妙ですが……]

=== 1月14日 午前9時52分 DNSクエリ送信(2回目)

=== 1月27日 午後2時30分 DNSレスポンス受信(2回目)

=== 1月27日 午後6時56分 DNSクエリ送信(3回目)

=== 2月10日 午前9時35分 DNSレスポンス受信(3回目)

== TCP接続ハンドシェイク

まずはサーバーちゃんにご挨拶します。

=== 2月19日 午後3時27分 TCPハンドシェイク SYNパケット送信

=== 2月27日 午後6時30分 TCPハンドシェイク SYNパケット到達

=== 2月27日 午後8時11分 TCPハンドシェイク SYN+ACKパケット送信

=== 3月9日 午前3時43分 TCPハンドシェイク SYN+ACKパケット到達

=== 3月9日 午前4時5分 TCPハンドシェイク ACKパケット送信

=== 3月16日 午後3時21分 TCPハンドシェイク ACKパケット到達

== TLSハンドシェイク

TLSはTCPのようなコネクションの上で暗号技術を使って通信の機密性や完全性を確保するための仕組みです。
公開鍵暗号を用いて安全に交換した鍵を使って共通鍵暗号を行い、
暗号化されたコネクションをアプリケーションに提供します。

この記事は暗号理論の説明はいたしません。
なので、やり取りする情報のさらなる意味を知りたい方は、
ぜひ他の専門書を参照してください。

=== 3月9日 午後7時27分 ClientHello 送信

サーバーちゃんと秘密のやりとりをするために、


TLSそのものは暗号を行う方式ではなく、
別に定められた暗号方式を使って通信を行うための仕組みです。
そのため、TLSハンドシェイクではこの暗号方式を決めることがコアになります。

暗号というものは時間が立つに連れて、その弱点が明らかになったり、
コンピューターの性能向上とともに解読に必要な時間が短くなるなどで弱くなります。
そのため、情報セキュリティにおいて暗号を利用する際には、
使用する暗号方式を新しい物に移行できることが重要になります。
TLSは将来開発しうる暗号方式を受け入れる事ができる、
一種のフレームワークとして作られています。

今回はクライアントちゃんとサーバーちゃんとで暗号通信を行いますが、
当然ながらお互いに使える暗号方式は限られており、
事前にそれらはわかりません。
TLSではこの暗号方式を後述するCipherSuiteと呼ばれる16bitの識別子で取り扱います。
ClientHelloではクライアントちゃんが使うことができる好みのCipherSuiteのリストを
サーバーちゃんに送ります。

他にもバージョン番号やセッション再開(resumption)に使用する乱数、使用できる署名及びアルゴリズムのリストなどが送信されます。

==== CipherSuite

CipherSuiteとは鍵交換アルゴリズム(Key Exchange)、暗号アルゴリズム(Cipher)、メッセージ認証符号(MAC)の組み合わせであり、
16bitの識別子が割り振られている。
例えば TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384 は鍵交換にECDHE_RSA、
暗号にAES_256_GCM、メッセージ認証符号にSHA384を使用することを示している。

=== 3月19日 午前10時30分 ClientHello 受信

配達に10日もかかっています。おそらく津軽海峡を超えるのに時間がかかったのでしょう。

ClientHelloを受信したら、その中のCipherSuiteのリストや署名及びハッシュアルゴリズムのリストから実際に使用する方式を選定します。
この選定方法はアプリケーションによって異なり、管理者が設定を行えます。

=== 3月20日 午後7時20分 ServerHello, Certificate, ServerKeyExchange 送信

サーバーちゃんはクライアントちゃんからClientHelloを受けて、返事をします。
この返事は複数のメッセージで構成されることがあります。

ServerHelloでは実際に使用するCipherSuiteとセッション再開に使用する乱数などの情報を送ります。

Certificateではサーバー証明書、及びその証明書を検証するために必要な証明書をクライアントちゃんに送ります。
この証明書は先に決定したCipherSuiteで使えるものでなければならないです。
運用上、実際には証明書の方を認証局から発行してもらい、
簡単にいじれるものではないため、証明書に適合したCipherSuiteを選ぶことになります。
このCertificateは任意であるが、これを送らないということは、
通信先であるサーバーが、本当に想定されるサーバーなのかの手がかりを与えない、匿名な通信となります。
暗号に置いて通信相手が攻撃者などではなく、意図した通信相手であることを保証することは重要です。

ServerKeyExchangeでは公開鍵や署名をクライアントちゃんに送ります。
具体的なデータ内容などはCipherSuiteで指定した鍵交換アルゴリズムによってことなります。
このServerKeyExchangeは任意であり単純なRSAのように、
Certificateに公開鍵や署名を含む場合にはこのメッセージは送られません。
しかしながら、この公開鍵が署名と一体であるということは、
すべての通信で使いまわしているということであり、安全性に疑問が残ります。
将来的に公開鍵とセットである秘密鍵が破られた際に、後から秘匿性が失われる恐れがあります。
そのため、鍵を使い捨てにして前方秘匿性(forward secrecy)を確保することができる、
DHE等のアルゴリズムが推奨されています。
この使い捨ての鍵を証明書とは別に送信する際にServerKeyExchangeは使われます。

最後にServerHelloDoneを送ることで、返事を終わらせます。

== HTTPリクエストとレスポンス

== TLS終了アラート

== TCPコネクション切断

== 最後に

これは文章でした。
