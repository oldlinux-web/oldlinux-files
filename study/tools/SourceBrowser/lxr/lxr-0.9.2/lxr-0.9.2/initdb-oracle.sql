drop sequence filenum;
drop sequence symnum;
drop table indexes;
drop table usage;
drop table symbols;
drop table releases;
drop table status;
drop table files;

commit;

create sequence filenum;
create sequence symnum;

commit;

create table files ( 			
	filename	varchar2(250),
	revision	varchar2(250),
	fileid		number,
	constraint pk_files primary key (fileid)
);
alter table files add unique (filename, revision);
create index i_files on files(filename);

commit;

create table symbols (				
	symname		varchar2(250),
	symid		number,
	constraint pk_symbols primary key (symid)
);
alter table symbols add unique(symname);

commit;

create table indexes (
	symid		number,
	fileid		number,
	line		number,
	type		varchar2(250),
	relsym		number,
	constraint fk_indexes_fileid foreign key (fileid) references files(fileid),
	constraint fk_indexes_symid foreign key (symid) references symbols(symid),
	constraint fk_indexes_relsym foreign key (relsym) references symbols(symid)
);
create index i_indexes on indexes(symid);

commit;

create table releases (	
	fileid		number,
	release		varchar2(250),
	constraint pk_releases primary key (fileid,release),
	constraint fk_releases_fileid foreign key (fileid) references files(fileid)
);

commit;

create table status (
	fileid		number,
	status		number,
	constraint pk_status primary key (fileid),
	constraint fk_status_fileid foreign key (fileid) references files(fileid)
);

commit;

create table usage (				
	fileid		number,
	line		number,
	symid		number,
	constraint fk_usage_fileid foreign key (fileid) references files(fileid),
	constraint fk_usage_symid foreign key (symid) references symbols(symid)
);
create index i_usage on usage(symid);

commit;