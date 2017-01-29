{
  config: {
    title: 'SunPro 2016技術書典',
    author: 'SunPro',
    email: 'admin@sunpro.io',
    description: '2016年、インターネットが日本中のあらゆる人間に行き渡るようになってから、すでに10年単位の時間が経過しています。今日においてインターネットを支えるネットワーク技術が重要であることは言うまでもありませんが、実際にネットワークでどのタイミングで何が起こり、どれくらいの時間が費やされるのかということを身を持って体感している人は、たとえネットワークに精通している人でも少ないのではないでしょうか？この記事では、1秒というわずかな時間を1年にまで拡大し、ネットワーク上で何が起こっているかを人間スケールでざっくりと解説していきます。',
  },
  page: {
    url: '',
    plain_title: '',
    subtitle: 'hakatashi / Mine02C4',
    content: fs.readFileSync('../hakatashi.html'),
    posts: [],
  },
  theme: {
    'navbar-links': {
      'Home': undefined,
      'GitHub': 'https://github.com/hakatashi/SunPro-techbookfest',
    },
    footer: [],
    avatar: '/../../favicon.png',
    comment: {
      enable: false,
    },
    highlight: {
      enable: true,
    },
    header: {
      title: 'SunPro <wbr>2016<wbr>技術書典',
      motto: '技術系よろず同人誌 (2016年6月発行)',
      bigimgs: [{
        src: '/../../cover.jpg',
        desc: '表紙',
      }],
    },
  },
  url_for: (path) => {
    if (path === undefined) {
      return './';
    } else if (path.startsWith('http')) {
      return path;
    } else {
      return `beautiful-hexo/source${path}`;
    }
  },
}
