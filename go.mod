module naive.systems/analyzer

go 1.18

require (
	github.com/bmatcuk/doublestar/v4 v4.0.2
	github.com/golang/glog v1.0.0
	github.com/google/shlex v0.0.0-20191202100458-e7afc7fbc510
	github.com/google/uuid v1.3.0
	github.com/hhatto/gocloc v0.4.2
	github.com/lib/pq v1.10.6
	github.com/libgit2/git2go/v33 v33.0.7
	github.com/securego/gosec/v2 v2.11.0
	golang.org/x/exp v0.0.0-20220518171630-0b5c67f07fdf
	golang.org/x/text v0.3.7
	golang.org/x/tools v0.1.11-0.20220513221640-090b14e8501f
	google.golang.org/protobuf v1.27.1
	gopkg.in/yaml.v2 v2.4.0
	honnef.co/go/tools v0.3.2
)

require (
	github.com/BurntSushi/toml v0.4.1 // indirect
	github.com/go-enry/go-enry/v2 v2.7.2 // indirect
	github.com/go-enry/go-oniguruma v1.2.1 // indirect
	github.com/nbutton23/zxcvbn-go v0.0.0-20210217022336-fa2cb2858354 // indirect
	golang.org/x/crypto v0.0.0-20220525230936-793ad666bf5e // indirect
	golang.org/x/exp/typeparams v0.0.0-20220218215828-6cf2b201936e // indirect
	golang.org/x/mod v0.6.0-dev.0.20220419223038-86c51ed26bb4 // indirect
	golang.org/x/sys v0.0.0-20211216021012-1d35b9e2eb4e // indirect
)

replace (
	github.com/golang/glog => ./third_party/github.com/golang/glog
	github.com/hhatto/gocloc => ./third_party/github.com/hhatto/gocloc
	github.com/libgit2/git2go/v33 => ./third_party/github.com/libgit2/git2go.v33
	github.com/securego/gosec/v2 => ./third_party/github.com/securego/gosec.v2
)
